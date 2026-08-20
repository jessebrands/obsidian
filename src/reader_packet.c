/*
 * reader_packet.c: packet reader
 */

#include <assert.h>
#include <string.h>

#include "reader.h"

static size_t
avail(struct pkt_reader const* r) {
    assert(r != NULL);
    return r->cur - r->pos;
}

static void
skip(struct pkt_reader* r, size_t const cnt) {
    assert(r != NULL);
    assert(r->pos + cnt <= r->cur);
    r->pos += cnt;
    r->offset += cnt;
}

static mc_byte
read_byte(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert((r->pos + sizeof(mc_byte)) <= r->cur);

    mc_byte const b = r->buffer[r->pos];
    r->pos += sizeof b;
    r->offset += sizeof b;
    return b;
}

static mc_word
read_word(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    mc_word x;
    uint8_t const* b = &r->buffer[r->pos];

    /* copy and swap */
    assert((r->pos + sizeof x) <= r->cur);
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->offset += sizeof x;
    return __builtin_bswap16(x);
}

static mc_dword
read_dword(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    mc_dword x;
    uint8_t const* b = &r->buffer[r->pos];

    /* copy and swap */
    assert((r->pos + sizeof x) <= r->cur);
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->offset += sizeof x;
    return __builtin_bswap32(x);
}

static mc_qword
read_qword(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    mc_qword x;
    uint8_t const* b = &r->buffer[r->pos];

    /* copy and swap */
    assert((r->pos + sizeof x) <= r->cur);
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->offset += sizeof x;
    return __builtin_bswap64(x);
}

static mc_int
read_int(struct pkt_reader* r) {
    return (mc_int) read_dword(r);
}

static mc_float
read_float(struct pkt_reader* r) {
    mc_dword const x = read_dword(r);
    mc_float f;

    /* coerce into float */
    memcpy(&f, &x, sizeof f);
    return f;
}

static mc_double
read_double(struct pkt_reader* r) {
    mc_qword const x = read_qword(r);
    mc_double d;

    /* coerce into float */
    memcpy(&d, &x, sizeof d);
    return d;
}

static mc_bool
read_bool(struct pkt_reader* r) {
    mc_byte const b = read_byte(r);
    /* ensure we're truly handling a bool */
    assert(b == 0 || b == 1);
    return b != 0;
}

static mc_byte const*
read_bytes(struct pkt_reader* r, size_t const len) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    assert((r->pos + len) <= r->cur);
    uint8_t const* b = &r->buffer[r->pos];
    r->pos += len;
    r->offset += len;
    return b;
}

size_t
read_packet_id(struct pkt_reader* r, mc_byte* id) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(id != NULL);

    if (avail(r) < sizeof *id) {
        r->overflow = true;
        return sizeof *id;
    }

    *id = read_byte(r);
    return 0;
}

size_t
read_srv_pkt_auth(struct pkt_reader* r, struct srv_pkt_auth* pkt) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(pkt != NULL);

    if (avail(r) < SRV_PKT_AUTH_SIZE) {
        r->overflow = true;
        return SRV_PKT_AUTH_SIZE;
    }

    *pkt = (struct srv_pkt_auth){
        .unknown0 = read_dword(r),
        .unknown1 = read_dword(r),
    };
    return 0;
}

size_t
read_srv_pkt_message(struct pkt_reader* r, struct srv_pkt_message* pkt) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(pkt != NULL);

    /* need at least the message length */
    if (avail(r) < SRV_PKT_MESSAGE_MIN_SIZE) {
        r->overflow = true;
        return SRV_PKT_MESSAGE_MIN_SIZE;
    }

    /* do we have the full string available? */
    mc_word const len = read_word(r);
    if (avail(r) < len) {
        r->overflow = true;
        return SRV_PKT_MESSAGE_MIN_SIZE + len;
    }

    *pkt = (struct srv_pkt_message){
        .length = len,
        .bytes = read_bytes(r, len),
    };
    return 0;
}

size_t
read_srv_pkt_full_position(struct pkt_reader* r,
                           struct srv_pkt_full_position* pkt) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(pkt != NULL);

    if (avail(r) < SRV_PKT_FULL_POSITION_SIZE) {
        r->overflow = true;
        return SRV_PKT_FULL_POSITION_SIZE;
    }

    *pkt = (struct srv_pkt_full_position){
        .x = read_double(r),
        .head_y = read_double(r),
        .y = read_double(r),
        .z = read_double(r),
        .rotation = read_float(r),
        .head_pitch = read_float(r),
        .grounded = read_bool(r),
    };
    return 0;
}

size_t
read_srv_pkt_0x15(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    if (avail(r) < SRV_PKT_0x15_SIZE) {
        r->overflow = true;
    }

    /* no overflow means this gets skipped */
    skip(r, SRV_PKT_0x15_SIZE);
    return 0;
}

size_t
read_srv_pkt_entity(struct pkt_reader* r, struct srv_pkt_entity* pkt) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(pkt != NULL);

    if (avail(r) < SRV_PKT_ENTITY_SIZE) {
        r->overflow = true;
        return SRV_PKT_ENTITY_SIZE;
    }

    *pkt = (struct srv_pkt_entity) {
        .id = read_dword(r),
    };
    return 0;
}

size_t
read_srv_pkt_chunk(struct pkt_reader* r, struct srv_pkt_chunk* pkt) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(pkt != NULL);

    if (avail(r) < SRV_PKT_CHUNK_SIZE) {
        r->overflow = true;
        return SRV_PKT_CHUNK_SIZE;
    }

    *pkt = (struct srv_pkt_chunk) {
        .x = read_int(r),
        .z = read_int(r),
        .load = read_bool(r),
    };
    return 0;
}

size_t
read_srv_pkt_chunk_data(struct pkt_reader* r, struct srv_pkt_chunk_data* pkt) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(pkt != NULL);

    if (avail(r) < SRV_PKT_CHUNK_DATA_MIN_SIZE) {
        r->overflow = true;
        return SRV_PKT_CHUNK_DATA_MIN_SIZE;
    }

    skip(r, 10); /* unknown data */

    /* read chunk extents */
    mc_byte const x = read_byte(r);
    mc_byte const y = read_byte(r);
    mc_byte const z = read_byte(r);

    /* uncompressed data */
    mc_dword const compressed_size = read_dword(r);
    if (avail(r) < compressed_size) {
        r->overflow = true;
        return SRV_PKT_CHUNK_DATA_MIN_SIZE + compressed_size;
    }

    *pkt = (struct srv_pkt_chunk_data){
        .x = x,
        .y = y,
        .z = z,
        .compressed_size = compressed_size,
        .data = read_bytes(r, compressed_size),
    };
    return 0;
}

size_t
read_srv_pkt_0x35(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    if (avail(r) < SRV_PKT_0x35_SIZE) {
        r->overflow = true;
    }

    /* no overflow means this gets skipped */
    skip(r, SRV_PKT_0x35_SIZE);
    return 0;
}
