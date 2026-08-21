#include <assert.h>
#include <string.h>

#include "buffer.h"

#define read_head(r) (&r->data[r->pos])
#define read_avail(r) (r->cur - r->pos)
#define read_has(r, n) ((r->pos + n) <= r->cur)

static void
skip(struct pkt_buffer* r, size_t const cnt) {
    assert(r != NULL);
    assert(read_has(r, cnt));
    r->pos += cnt;
    r->in_total += cnt;
}

static mc_byte
read_byte(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(read_has(r, sizeof(mc_byte)));

    mc_byte const b = read_head(r)[0];
    r->pos += sizeof b;
    r->in_total += sizeof b;
    return b;
}

static mc_word
read_word(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    mc_word x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return __builtin_bswap16(x);
}

static mc_dword
read_dword(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    mc_dword x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return __builtin_bswap32(x);
}

static mc_qword
read_qword(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    mc_qword x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return __builtin_bswap64(x);
}

static mc_int
read_int(struct pkt_buffer* r) {
    return (mc_int) read_dword(r);
}

static mc_float
read_float(struct pkt_buffer* r) {
    mc_dword const x = read_dword(r);
    mc_float f;

    /* coerce into float */
    memcpy(&f, &x, sizeof f);
    return f;
}

static mc_double
read_double(struct pkt_buffer* r) {
    mc_qword const x = read_qword(r);
    mc_double d;

    /* coerce into float */
    memcpy(&d, &x, sizeof d);
    return d;
}

static mc_bool
read_bool(struct pkt_buffer* r) {
    mc_byte const b = read_byte(r);
    /* ensure we're truly handling a bool */
    assert(b == 0 || b == 1);
    return b != 0;
}

static mc_byte const*
read_bytes(struct pkt_buffer* r, size_t const len) {
    assert(r != NULL);
    assert(r->data != NULL);

    assert(read_has(r, len));
    uint8_t const* b = read_head(r);
    r->pos += len;
    r->in_total += len;
    return b;
}

size_t
read_packet_id(struct pkt_buffer* r, mc_byte* id) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(id != NULL);

    if (read_avail(r) < sizeof *id) {
        r->overflow = true;
        return sizeof *id;
    }

    *id = read_byte(r);
    return 0;
}

size_t
read_srv_pkt_auth(struct pkt_buffer* r, struct srv_pkt_auth* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (read_avail(r) < SRV_PKT_AUTH_SIZE) {
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
read_srv_pkt_message(struct pkt_buffer* r, struct srv_pkt_message* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    /* need at least the message length */
    if (!read_has(r, SRV_PKT_MESSAGE_MIN_SIZE)) {
        r->overflow = true;
        return SRV_PKT_MESSAGE_MIN_SIZE;
    }

    /* do we have the full string available? */
    mc_word const len = read_word(r);
    if (read_avail(r) < len) {
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
read_srv_pkt_full_position(struct pkt_buffer* r,
                           struct srv_pkt_full_position* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_FULL_POSITION_SIZE)) {
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
read_srv_pkt_0x15(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    if (!read_has(r, SRV_PKT_0x15_SIZE)) {
        r->overflow = true;
    }

    /* no overflow means this gets skipped */
    skip(r, SRV_PKT_0x15_SIZE);
    return 0;
}

size_t
read_srv_pkt_entity(struct pkt_buffer* r, struct srv_pkt_entity* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENTITY_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENTITY_SIZE;
    }

    *pkt = (struct srv_pkt_entity) {
        .id = read_dword(r),
    };
    return 0;
}

size_t
read_srv_pkt_chunk(struct pkt_buffer* r, struct srv_pkt_chunk* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_CHUNK_SIZE)) {
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
read_srv_pkt_chunk_data(struct pkt_buffer* r, struct srv_pkt_chunk_data* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_CHUNK_DATA_MIN_SIZE)) {
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
    if (read_avail(r) < compressed_size) {
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
read_srv_pkt_0x35(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    if (!read_has(r, SRV_PKT_0x35_SIZE)) {
        r->overflow = true;
    }

    /* no overflow means this gets skipped */
    skip(r, SRV_PKT_0x35_SIZE);
    return 0;
}
