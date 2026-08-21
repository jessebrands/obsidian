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
        case SRV_RECEIVE_ITEM: return "RECEIVE_ITEM";
        case SRV_SPAWN_PLAYER: return "SPAWN_PLAYER";
        case SRV_SPAWN_ITEM: return "SPAWN_ITEM";
        case SRV_ENT_PICKUP: return "ENT_PICKUP";
        case SRV_ENT_DESTROY: return "ENT_DESTROY";
        case SRV_ENT_ALIVE: return "ENT_ALIVE";
        case SRV_ENT_MOVE: return "ENT_MOVE";
        case SRV_ENT_LOOK: return "ENT_LOOK";
        case SRV_ENT_MOVE_LOOK: return "ENT_MOVE_LOOK";
        case SRV_ENT_FULL_POS: return "ENT_FULL_POS";
        case SRV_CHUNK: return "CHUNK";
        case SRV_CHUNK_DATA: return "CHUNK_DATA";
        default: return "UNKNOWN";
    }
}

static size_t
print_srv_pkt_heartbeat(struct pkt_buffer* r) {
    size_t const offset = r->in_total - 1;
    printf("%08zx  %02x:%-15s\n", offset, 0x00, srv_pkt_name(0x00));
    return 0;
}

static size_t
print_srv_pkt_auth(struct pkt_buffer* r) {
    struct srv_pkt_auth pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_auth(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x01, srv_pkt_name(0x01));
    printf("{ unknown0: %" PRIi32 ", unknown1: %" PRIi32 " }\n",
           pkt.unknown0, pkt.unknown1);
    return 0;
}

static size_t
print_srv_pkt_message(struct pkt_buffer* r) {
    struct srv_pkt_message pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_message(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x03, srv_pkt_name(0x03));
    printf("\"%.*s\"\n", pkt.length, (char const*) pkt.bytes);
    return 0;
}

static size_t
print_srv_pkt_full_position(struct pkt_buffer* r) {
    struct srv_pkt_full_position pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_full_position(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x0d, srv_pkt_name(0x0d));
    printf("{ x: %.2f, head_y: %.2f, y: %.2f,  z: %.2f, rotation: %.2f, head_pitch: %.2f, grounded: %s }\n",
           pkt.x, pkt.head_y, pkt.y, pkt.z, pkt.rotation, pkt.head_pitch, pkt.grounded ? "true" : "false");
    return 0;
}

static size_t
print_srv_pkt_receive_item(struct pkt_buffer* r) {
    struct srv_pkt_receive_item pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_receive_item(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x11, srv_pkt_name(0x11));
    printf("{ item: %d, count: %d, durability: %d }\n",
           pkt.item, pkt.count, pkt.durability);
    return 0;
}

static size_t
print_srv_pkt_spawn_player(struct pkt_buffer* r) {
    struct srv_pkt_spawn_player pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_spawn_player(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    double const x = (double) pkt.x / 32;
    double const y = (double) pkt.y / 32;
    double const z = (double) pkt.z / 32;
    float const yaw = ((float) pkt.yaw / 128.0f) * 180.0f;
    float const pitch = ((float) pkt.pitch / 128.0f) * 180.0f;

    printf("%08zx  %02x:%-15s  ", offset, 0x14, srv_pkt_name(0x14));
    printf("{ entity: %08x, name: \"%.*s\", x: %.1f, y: %.1f, z: %.1f, yaw: %.1f, pitch %1.f, item: %d }\n",
           pkt.entity, pkt.name_length, (char const*) pkt.name,
           x, y, z, yaw, pitch, pkt.item);
    return 0;
}

static size_t
print_srv_pkt_spawn_item(struct pkt_buffer* r) {
    struct srv_pkt_spawn_item pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_spawn_item(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    double const x = (double) pkt.x / 32;
    double const y = (double) pkt.y / 32;
    double const z = (double) pkt.z / 32;
    float const yaw = ((float) pkt.yaw / 128.0f) * 180.0f;
    float const pitch = ((float) pkt.pitch / 128.0f) * 180.0f;

    printf("%08zx  %02x:%-15s  ", offset, 0x15, srv_pkt_name(0x15));
    printf("{ entity: %08x, item: %04x, count: %d, x: %.1f, y: %.1f, z: %.1f, yaw: %.1f, pitch: %.1f, unknown: %d }\n",
           pkt.entity, pkt.item, pkt.count, x, y, z, yaw, pitch, pkt.unknown);
    return 0;
}

static size_t
print_srv_pkt_ent_pickup(struct pkt_buffer* r) {
    struct srv_pkt_ent_pickup pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_ent_pickup(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x16, srv_pkt_name(0x16));
    printf("{ entity: %08x, receiver: %08x }\n", pkt.item, pkt.entity);
    return 0;
}

static size_t
print_srv_pkt_ent_destroy(struct pkt_buffer* r) {
    struct srv_pkt_ent_destroy pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_ent_destroy(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x1d, srv_pkt_name(0x1d));
    printf("{ entity: %08x }\n", pkt.entity);
    return 0;
}

static size_t
print_srv_pkt_ent_alive(struct pkt_buffer* r) {
    struct srv_pkt_ent_alive pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_ent_alive(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x1e, srv_pkt_name(0x1e));
    printf("{ entity: %08x }\n", pkt.entity);
    return 0;
}

static size_t
print_srv_pkt_ent_move(struct pkt_buffer* r) {
    struct srv_pkt_ent_move pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_ent_move(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    double const x = (double) pkt.x / 32.0;
    double const y = (double) pkt.y / 32.0;
    double const z = (double) pkt.z / 32.0;

    printf("%08zx  %02x:%-15s  ", offset, 0x1f, srv_pkt_name(0x1f));
    printf("{ entity: %08x, x: %.1f, y: %.1f, z: %.1f }\n",
           pkt.id, x, y, z);
    return 0;
}

static size_t
print_srv_pkt_ent_look(struct pkt_buffer* r) {
    struct srv_pkt_ent_look pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_ent_look(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    float const yaw = ((float) pkt.yaw / 256.0f) * 360.0f;
    float const pitch = ((float) pkt.pitch / 256.0f) * 360.0f;

    printf("%08zx  %02x:%-15s  ", offset, 0x20, srv_pkt_name(0x20));
    printf("{ entity: %08x, yaw: %.1f, pitch: %.1f }\n", pkt.id, yaw, pitch);
    return 0;
}

static size_t
print_srv_pkt_ent_move_look(struct pkt_buffer* r) {
    struct srv_pkt_ent_move_look pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_ent_move_look(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    double const x = (double) pkt.x / 32.0;
    double const y = (double) pkt.y / 32.0;
    double const z = (double) pkt.z / 32.0;
    float const yaw = ((float) pkt.yaw / 256.0f) * 360.0f;
    float const pitch = ((float) pkt.pitch / 256.0f) * 360.0f;

    printf("%08zx  %02x:%-15s  ", offset, 0x21, srv_pkt_name(0x21));
    printf("{ entity: %08x, x: %.1f, y: %.1f, z: %.1f, yaw: %.1f, pitch: %.1f }\n",
           pkt.id, x, y, z, yaw, pitch);
    return 0;
}

static size_t
print_srv_pkt_ent_full_pos(struct pkt_buffer* r) {
    struct srv_pkt_ent_full_pos pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_ent_full_pos(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    /* entity positions and rotations need conversion */
    double const x = (double) pkt.x / 32;
    double const y = (double) pkt.y / 32;
    double const z = (double) pkt.z / 32;
    float const yaw = ((float) pkt.yaw / 256.0f) * 360.0f;
    float const pitch = ((float) pkt.pitch / 256.0f) * 360.0f;

    printf("%08zx  %02x:%-15s  ", offset, 0x22, srv_pkt_name(0x22));
    printf("{ entity: %08x, x: %.1f, y: %.1f, z: %.1f, yaw: %.1f, pitch: %.1f }\n",
           pkt.id, x, y, z, yaw, pitch);
    return 0;
}

static size_t
print_srv_pkt_chunk(struct pkt_buffer* r) {
    struct srv_pkt_chunk pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_chunk(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x32, srv_pkt_name(0x32));
    printf("{ x: %d, z: %d, load: %s }\n",
           pkt.chunk.x, pkt.chunk.z, pkt.load ? "true" : "false");
    return 0;
}

static size_t
print_srv_pkt_chunk_data(struct pkt_buffer* r) {
    struct srv_pkt_chunk_data pkt;
    size_t const offset = r->in_total - 1;
    size_t const wanted = read_srv_pkt_chunk_data(r, &pkt);
    if (wanted != 0) {
        return wanted;
    }

    printf("%08zx  %02x:%-15s  ", offset, 0x33, srv_pkt_name(0x33));
    printf("{ origin( %d, %d, %d ), extent( %d, %d, %d ) "
           "size %" PRIi32", data: ... }\n",
           pkt.origin.x, pkt.origin.y, pkt.origin.z,
           pkt.extent.x, pkt.extent.y, pkt.extent.z,
           pkt.compressed_size);

    return 0;
}

static size_t
print_srv_pkt_0x34(struct pkt_buffer* r) {
    size_t const offset = r->in_total - 1;
    size_t const start = r->in_total;
    size_t const wanted = read_srv_pkt_0x34(r);
    if (wanted != 0) {
        return wanted;
    }

    size_t const end = r->in_total;
    printf("%08zx  %02x:%-15s  ", offset, 0x34, srv_pkt_name(0x34));
    printf("skipping %zu bytes\n", end - start);
    return 0;
}

static size_t
print_srv_pkt_0x35(struct pkt_buffer* r) {
    size_t const offset = r->in_total - 1;
    size_t const start = r->in_total;
    size_t const wanted = read_srv_pkt_0x35(r);
    if (wanted != 0) {
        return wanted;
    }

    size_t const end = r->in_total;
    printf("%08zx  %02x:%-15s  ", offset, 0x35, srv_pkt_name(0x35));
    printf("skipping %zu bytes\n", end - start);
    return 0;
}

static size_t
read_packet(struct pkt_buffer* r, mc_byte const pkt_id) {
    switch (pkt_id) {
        case SRV_HEARTBEAT:
            return print_srv_pkt_heartbeat(r);

        case SRV_AUTH:
            return print_srv_pkt_auth(r);

        case SRV_MESSAGE:
            return print_srv_pkt_message(r);

        case SRV_FULL_POSITION:
            return print_srv_pkt_full_position(r);

        case SRV_RECEIVE_ITEM:
            return print_srv_pkt_receive_item(r);

        case SRV_SPAWN_PLAYER:
            return print_srv_pkt_spawn_player(r);

        case SRV_SPAWN_ITEM:
            return print_srv_pkt_spawn_item(r);

        case SRV_ENT_PICKUP:
            return print_srv_pkt_ent_pickup(r);

        case SRV_ENT_DESTROY:
            return print_srv_pkt_ent_destroy(r);

        case SRV_ENT_ALIVE:
            return print_srv_pkt_ent_alive(r);

        case SRV_ENT_MOVE:
            return print_srv_pkt_ent_move(r);

        case SRV_ENT_LOOK:
            return print_srv_pkt_ent_look(r);

        case SRV_ENT_MOVE_LOOK:
            return print_srv_pkt_ent_move_look(r);

        case SRV_ENT_FULL_POS:
            return print_srv_pkt_ent_full_pos(r);

        case SRV_CHUNK:
            return print_srv_pkt_chunk(r);

        case SRV_CHUNK_DATA:
            return print_srv_pkt_chunk_data(r);

        case SRV_0x34:
            return print_srv_pkt_0x34(r);

        case SRV_0x35:
            return print_srv_pkt_0x35(r);

        default:
            fprintf(stderr, "unknown packet 0x%02x\n", pkt_id);
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
