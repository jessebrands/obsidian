/*
 * dissect.c: protocol dissector
 */

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include "packet/buffer.h"

static char const*
srv_pkt_name(enum srv_pkt const pkt) {
    switch (pkt) {
        case SRV_HEARTBEAT: return "HEARTBEAT";
        case SRV_AUTH: return "AUTH";
        case SRV_MESSAGE: return "MESSAGE";
        case SRV_FULL_POSITION: return "FULL_POS";
        case SRV_ENTITY: return "ENTITY";
        case SRV_CHUNK: return "CHUNK";
        case SRV_CHUNK_DATA: return "CHUNK_DATA";
        default: return "UNKNOWN";
    }
}

static size_t
print_srv_pkt_heartbeat(struct pkt_buffer* r) {
    printf("\n");
    return 0;
}

static size_t
print_srv_pkt_auth(struct pkt_buffer* r) {
    struct srv_pkt_auth pkt;
    size_t const wanted = read_srv_pkt_auth(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("{ unknown0: %" PRIu32 ", unknown1: %" PRIu32 " }\n",
           pkt.unknown0, pkt.unknown1);
    return 0;
}

static size_t
print_srv_pkt_message(struct pkt_buffer* r) {
    struct srv_pkt_message pkt;
    size_t const wanted = read_srv_pkt_message(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("\"%.*s\"\n", pkt.length, (char const*) pkt.bytes);
    return 0;
}

static size_t
print_srv_pkt_full_position(struct pkt_buffer* r) {
    struct srv_pkt_full_position pkt;
    size_t const wanted = read_srv_pkt_full_position(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("{ x: %.2f, head_y: %.2f, y: %.2f,  z: %.2f, rotation: %.2f, head_pitch: %.2f, grounded: %s }\n",
           pkt.x, pkt.head_y, pkt.y, pkt.z, pkt.rotation, pkt.head_pitch, pkt.grounded ? "true" : "false");
    return 0;
}

static size_t
print_srv_pkt_0x15(struct pkt_buffer* r) {
    size_t const wanted = read_srv_pkt_0x15(r);
    printf("skipping %u bytes\n", SRV_PKT_0x15_SIZE);
    return wanted;
}

static size_t
print_srv_pkt_entity(struct pkt_buffer* r) {
    struct srv_pkt_entity pkt;
    size_t const wanted = read_srv_pkt_entity(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("{ id: %08" PRIx32 " }\n", pkt.id);
    return 0;
}

static size_t
print_srv_pkt_chunk(struct pkt_buffer* r) {
    struct srv_pkt_chunk pkt;
    size_t const wanted = read_srv_pkt_chunk(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("{ x: %d, z: %d, load: %s }\n",
           pkt.x, pkt.z, pkt.load ? "true" : "false");
    return 0;
}

static size_t
print_srv_pkt_chunk_data(struct pkt_buffer* r) {
    struct srv_pkt_chunk_data pkt;
    size_t const wanted = read_srv_pkt_chunk_data(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("{ unknown, x: %u, y: %u, z: %u, compressed_size %" PRIu32", data: ... }\n",
           pkt.x, pkt.y, pkt.z, pkt.compressed_size);
    return 0;
}

static size_t
print_srv_pkt_0x34(struct pkt_buffer* r) {
    size_t const start = r->pos;
    size_t const wanted = read_srv_pkt_0x34(r);
    size_t const end = r->pos;
    printf("skipping %zu bytes\n", end - start);
    return wanted;
}

static size_t
print_srv_pkt_0x35(struct pkt_buffer* r) {
    size_t const wanted = read_srv_pkt_0x35(r);
    printf("skipping %u bytes\n", SRV_PKT_0x35_SIZE);
    return wanted;
}

static size_t
read_packet(struct pkt_buffer* r, mc_byte const pkt_id) {
    size_t const offset = r->in_total - 1; /* header byte */
    printf("%08zx  %02x:%-12s  ", offset, pkt_id, srv_pkt_name(pkt_id));

    switch (pkt_id) {
        case SRV_HEARTBEAT:
            return print_srv_pkt_heartbeat(r);

        case SRV_AUTH:
            return print_srv_pkt_auth(r);

        case SRV_MESSAGE:
            return print_srv_pkt_message(r);

        case SRV_FULL_POSITION:
            return print_srv_pkt_full_position(r);

        case SRV_0x15:
            return print_srv_pkt_0x15(r);

        case SRV_ENTITY:
            return print_srv_pkt_entity(r);

        case SRV_CHUNK:
            return print_srv_pkt_chunk(r);

        case SRV_CHUNK_DATA:
            return print_srv_pkt_chunk_data(r);

        case SRV_0x34:
            return print_srv_pkt_0x34(r);

        case SRV_0x35:
            return print_srv_pkt_0x35(r);

        default:
            fprintf(stderr, "error: unknown packet %02X\n", pkt_id);
            exit(EXIT_FAILURE);
    }
}

static size_t
next_packet(struct pkt_buffer* r) {
    mc_byte pkt_id;
    read_packet_id(r, &pkt_id);
    if (r->overflow) {
        return 1;
    }

    size_t const wanted = read_packet(r, pkt_id);
    if (r->overflow) {
        return 1 + wanted;
    }

    return wanted;
}

static void
dissect_stream(FILE* stream) {
    assert(stream != NULL);

    struct pkt_buffer r = {0};
    if (pkt_buffer_init(&r, 128) == NULL) {
        fprintf(stderr, "error: could not allocate reader buffer\n");
        exit(EXIT_FAILURE);
    }

    while (!feof(stream)) {
        /* drop unneeded bytes first */
        pkt_buffer_drop(&r);

        /* read as many bytes as we can */
        if (pkt_buffer_fread(&r, stream) == 0 && feof(stream)) {
            fprintf(stderr, "error: unexpected EOF\n");
            exit(EXIT_FAILURE);
        }

        /* attempt to read the next packet */
        size_t const start = r.pos;
        size_t const offset = r.in_total;
        size_t const needed = next_packet(&r);

        /* if we have overflow, then the read failed */
        if (r.overflow) {
            /* rewind to the start of the packet */
            r.pos = start;
            r.in_total = offset;
            r.overflow = false;

            /* can this packet fit in our buffer? */
            if (needed > r.capacity) {
                printf("growing buffer and retrying...\n");
                if (pkt_buffer_resize(&r, needed) == NULL) {
                    fprintf(stderr, "error: failed to grow buffer\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

static void
dissect(char const* filename) {
    assert(filename != NULL);

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return;
    }

    dissect_stream(file);
    fclose(file);
}

int
main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: dissect FILE\n");
        return EXIT_FAILURE;
    }

    char const* filename = argv[1];
    dissect(filename);

    printf("Reached end of stream, goodbye!! :-)\n");
    return EXIT_SUCCESS;
}
