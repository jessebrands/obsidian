/*
 * dissect.c: protocol dissector
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE (1024 * 16)

#define IDX(x) ((x) % BUFFER_SIZE)

typedef uint8_t mc_byte;
typedef uint16_t mc_word;
typedef uint32_t mc_dword;
typedef uint64_t mc_qword;
typedef float mc_float;
typedef double mc_double;
typedef bool mc_bool;

struct reader {
    uint8_t pkt_id;
    uint8_t buffer[BUFFER_SIZE];

    bool overflow; /* insufficient data flag */
    size_t pos; /* last complete read position */
    size_t peek_pos; /* partial seek position */
    size_t cursor; /* where new bytes are read */
};

/*
 * Sent to client upon connecting.
 */
struct srv_packet_0x01 {
    mc_dword unknown0;
    mc_dword unknown1;
};

struct srv_packet_0x03 {
    mc_word length;
    mc_byte const* bytes;
};

struct srv_packet_0x0d {
    mc_double x;
    mc_double y;
    mc_double head_y;
    mc_double z;
    mc_float rotation;
    mc_float head_pitch;
    mc_bool grounded;
};

struct srv_packet_0x1e {
    mc_dword unknown;
};

/*
 * Bombards the client with these after they connect.
 */
struct srv_packet_0x32 {
    mc_dword chunk_x;
    mc_dword chunk_z;
    mc_bool load;
};

static mc_byte
peek_byte(struct reader* r) {
    assert(r != NULL);

    /* end of buffer reached? */
    if (r->overflow || r->peek_pos + 1 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    /* we're okay, so keep reading */
    size_t const pos = IDX(r->peek_pos);
    r->peek_pos++;
    return r->buffer[pos];
}

static mc_byte const*
peek_string(struct reader* r, mc_dword const length) {
    assert(r != NULL);

    /* end of buffer reached? */
    if (r->overflow || r->peek_pos + length > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos += length;
    return &r->buffer[pos];
}

static mc_bool
peek_bool(struct reader* r) {
    assert(r != NULL);

    /* end of buffer reached? */
    if (r->overflow || r->peek_pos + 1 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos++;
    return r->buffer[pos] > 0;
}

static mc_word
peek_word(struct reader* r) {
    assert(r != NULL);
    if (r->overflow || r->peek_pos + 2 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos += 2;

    return (mc_word) r->buffer[IDX(pos + 0)] << 8
           | (mc_word) r->buffer[IDX(pos + 1)];
}

static mc_dword
peek_dword(struct reader* r) {
    assert(r != NULL);
    if (r->overflow || r->peek_pos + 4 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos += 4;

    return (mc_dword) r->buffer[IDX(pos)] << 24
           | (mc_dword) r->buffer[IDX(pos + 1)] << 16
           | (mc_dword) r->buffer[IDX(pos + 2)] << 8
           | (mc_dword) r->buffer[IDX(pos + 3)];
}

static mc_qword
peek_qword(struct reader* r) {
    assert(r != NULL);
    if (r->overflow || r->peek_pos + 8 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos += 8;

    return (mc_qword) r->buffer[IDX(pos + 0)] << 56
           | (mc_qword) r->buffer[IDX(pos + 1)] << 48
           | (mc_qword) r->buffer[IDX(pos + 2)] << 40
           | (mc_qword) r->buffer[IDX(pos + 3)] << 32
           | (mc_qword) r->buffer[IDX(pos + 4)] << 24
           | (mc_qword) r->buffer[IDX(pos + 5)] << 16
           | (mc_qword) r->buffer[IDX(pos + 7)] << 8
           | (mc_qword) r->buffer[IDX(pos + 8)];
}

static mc_float
peek_float(struct reader* r) {
    assert(r != NULL);
    if (r->overflow || r->peek_pos + 4 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos += 4;

    return (mc_float) ((mc_dword) r->buffer[IDX(pos)] << 24
                       | (mc_dword) r->buffer[IDX(pos + 1)] << 16
                       | (mc_dword) r->buffer[IDX(pos + 2)] << 8
                       | (mc_dword) r->buffer[IDX(pos + 3)]);
}


static mc_double
peek_double(struct reader* r) {
    assert(r != NULL);
    if (r->overflow || r->peek_pos + 8 > r->cursor) {
        r->overflow = true;
        return 0;
    }

    size_t const pos = IDX(r->peek_pos);
    r->peek_pos += 8;

    return (mc_double) ((mc_qword) r->buffer[IDX(pos + 0)] << 56
                        | (mc_qword) r->buffer[IDX(pos + 1)] << 48
                        | (mc_qword) r->buffer[IDX(pos + 2)] << 40
                        | (mc_qword) r->buffer[IDX(pos + 3)] << 32
                        | (mc_qword) r->buffer[IDX(pos + 4)] << 24
                        | (mc_qword) r->buffer[IDX(pos + 5)] << 16
                        | (mc_qword) r->buffer[IDX(pos + 7)] << 8
                        | (mc_qword) r->buffer[IDX(pos + 8)]);
}

/*
 * Keep alive
 */
static void
read_srv_packet_0x00(struct reader* r) {
    if (r->overflow) {
        return;
    }
    size_t const offset = r->pos;
    r->pos = r->peek_pos;

    printf("%08zX  %02X\n", offset, 0x00);
}

static void
read_srv_packet_0x01(struct reader* r) {
    struct srv_packet_0x01 const pkt = {
        .unknown0 = peek_dword(r),
        .unknown1 = peek_dword(r),
    };

    /* read OK? */
    if (r->overflow) {
        return;
    }
    size_t const offset = r->pos;
    r->pos = r->peek_pos;

    // Print the packet:
    printf("%08zX  %02X  { unknown0: %08X, unknown1: %08X }\n",
           offset, 0x01, pkt.unknown0, pkt.unknown1);
}

static void
read_srv_packet_0x03(struct reader* r) {
    mc_word const length = peek_word(r);
    struct srv_packet_0x03 const pkt = {
        .length = length,
        .bytes = peek_string(r, length),
    };

    if (r->overflow) {
        return;
    }
    size_t const offset = r->pos;
    r->pos = r->peek_pos;

    printf("%08zX  %02X  { message: \"%.*s\" }\n",
           offset, 0x03, pkt.length, (const char*) pkt.bytes);
}

/*
 * full player position
 */
static void
read_srv_packet_0x0d(struct reader* r) {
    assert(r != NULL);

    struct srv_packet_0x0d const pkt = {
        .x = peek_double(r),
        .y = peek_double(r),
        .head_y = peek_double(r),
        .z = peek_double(r),
        .rotation = peek_float(r),
        .head_pitch = peek_float(r),
        .grounded = peek_bool(r),
    };

    if (r->overflow) {
        return;
    }
    size_t const offset = r->pos;
    r->pos = r->peek_pos;

    printf("%08zX  %02X  { x: %f, y: %f, head_y: %f, z: %f, rotation: %f, head_pitch: %f, grounded: %s }\n",
           offset, 0x0d, pkt.x, pkt.y, pkt.head_y, pkt.z, pkt.rotation, pkt.head_pitch,
           pkt.grounded ? "true" : "false");
}

static void
read_srv_packet_0x15(struct reader* r) {
    assert(r != NULL);
    if (r->overflow || r->peek_pos + 22 > r->cursor) {
        r->overflow = true;
    }

    /* unknown packet, so just skip it */
    r->peek_pos += 22;
    r->pos = r->peek_pos;

    printf("%08zX  %02X  { ... }\n", r->pos, 0x15);
}

static void
read_srv_packet_0x1e(struct reader* r) {
    struct srv_packet_0x1e const pkt = {
        .unknown = peek_dword(r),
    };

    /* read OK? */
    if (r->overflow) {
        return;
    }
    size_t const offset = r->pos;
    r->pos = r->peek_pos;

    // Print the packet:
    printf("%08zX  %02X  { unknown: %08X }\n",
           offset, 0x1e, pkt.unknown);
}

static void
read_srv_packet_0x32(struct reader* r) {
    struct srv_packet_0x32 const pkt = {
        .chunk_x = peek_dword(r),
        .chunk_z = peek_dword(r),
        .load = peek_bool(r),
    };

    /* read OK? */
    if (r->overflow) {
        return;
    }
    size_t const offset = r->pos;
    r->pos = r->peek_pos;

    // Print the packet:
    printf("%08zX  %02X  { x: %d, z: %d, load: %s }\n",
           offset, 0x32, pkt.chunk_x, pkt.chunk_z, pkt.load ? "true" : "false");
}

static void
byte_dump(struct reader* r) {
    size_t const avail = r->cursor - r->pos;
    size_t const want = 65 < avail ? 65 : avail;

    size_t rem = want - 1;
    size_t pos = 1;

    while (rem != 0) {
        size_t const have = 16 < rem ? 16 : rem;

        printf("\t");
        for (size_t i = 0; i < have; ++i) {
            printf("%02X ", r->buffer[IDX(r->pos + pos + i)]);
        }
        printf("    ");
        for (size_t i = 0; i < have; ++i) {
            char const c = r->buffer[IDX(r->pos + pos + i)];
            if (c >= 0x20 && c <= 0x72) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("\n");

        pos += have;
        rem -= have;
    }


    printf("\n\n");
}

static bool
retry_read(struct reader* r) {
    /* record how many bytes we managed to read */
    size_t candidates[63] = {0};

    size_t const avail = r->cursor - r->pos;
    size_t const start = r->pos + 1;

    /* we'll give up after 63 tries */
    size_t const count = 63 < avail ? 63 : avail;
    for (size_t i = 0; i < count; ++i) {
        // Retry from every position.
        r->pos = start + i;
        r->peek_pos = start + 1;
    }

    return false;
}

static void
read_packet(struct reader* r, mc_byte const pkt_id) {
    switch (pkt_id) {
        case 0x00:
            read_srv_packet_0x00(r);
            return;

        case 0x01:
            read_srv_packet_0x01(r);
            return;

        case 0x03:
            read_srv_packet_0x03(r);
            return;

        case 0x0d:
            read_srv_packet_0x0d(r);
            return;

        case 0x15:
            read_srv_packet_0x15(r);
            return;

        case 0x1e:
            read_srv_packet_0x1e(r);
            return;

        case 0x32:
            read_srv_packet_0x32(r);
            return;

        default:
            /* we hit an unknown packet, so we dump our state */
            printf("unknown packet %02X at offset %08zX\n\n", pkt_id, r->pos);
            byte_dump(r);
            printf("giving up...\n");
            exit(EXIT_FAILURE);
    }
}

static void
parse_packet(struct reader* r) {
    /* reset peek position */
    r->peek_pos = r->pos;
    mc_byte const pkt_id = peek_byte(r);
    if (r->overflow) {
        fprintf(stderr, "overflowing on packet start, aborting!!!\n");
        exit(EXIT_FAILURE);
    }
    read_packet(r, pkt_id);
    fflush(stdout);
}

static void
dissect_stream(FILE* stream) {
    assert(stream != NULL);

    struct reader r = {0};
    while (!feof(stream)) {
        /* how many bytes do we want? */
        size_t const avail = sizeof r.buffer - (r.cursor - r.pos);
        size_t want = avail < sizeof r.buffer ? avail : sizeof r.buffer;

        /* calculate the real position in the buffer */
        size_t const cur = r.cursor % sizeof r.buffer;
        size_t const end = cur + want;

        /* if we'd read past the end of the buffer, cap it */
        if (end > sizeof r.buffer) {
            want -= (end - sizeof r.buffer);
        }

        /* advance the stream, read the buffer */
        r.cursor += fread(&r.buffer[cur], sizeof *r.buffer, want, stream);
        parse_packet(&r);
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

    return EXIT_SUCCESS;
}
