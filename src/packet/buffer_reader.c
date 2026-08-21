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

static mc_byte_t
read_byte(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(read_has(r, sizeof(mc_byte_t)));

    mc_byte_t const b = read_head(r)[0];
    r->pos += sizeof b;
    r->in_total += sizeof b;
    return b;
}

static mc_word_t
read_word(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    mc_word_t x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return __builtin_bswap16(x);
}

static mc_dword_t
read_dword(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    mc_dword_t x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return __builtin_bswap32(x);
}

static mc_qword_t
read_qword(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    mc_qword_t x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return __builtin_bswap64(x);
}

static mc_int_t
read_int(struct pkt_buffer* r) {
    return (mc_int_t) read_dword(r);
}

static mc_float_t
read_float(struct pkt_buffer* r) {
    mc_dword_t const x = read_dword(r);
    mc_float_t f;

    /* coerce into float */
    memcpy(&f, &x, sizeof f);
    return f;
}

static mc_double_t
read_double(struct pkt_buffer* r) {
    mc_qword_t const x = read_qword(r);
    mc_double_t d;

    /* coerce into float */
    memcpy(&d, &x, sizeof d);
    return d;
}

static mc_bool_t
read_bool(struct pkt_buffer* r) {
    mc_byte_t const b = read_byte(r);
    /* ensure we're truly handling a bool */
    assert(b == 0 || b == 1);
    return b != 0;
}

static mc_byte_t const*
read_bytes(struct pkt_buffer* r, size_t const len) {
    assert(r != NULL);
    assert(r->data != NULL);

    assert(read_has(r, len));
    uint8_t const* b = read_head(r);
    r->pos += len;
    r->in_total += len;
    return b;
}

static entity_id_t
read_entity_id(struct pkt_buffer* r) {
    mc_int_t const x = read_int(r);
    assert(x > 0 && x <= INT32_MAX);
    return (entity_id_t) x;
}

static item_id_t
read_item_id(struct pkt_buffer* r) {
    mc_word_t const x = read_word(r);
    assert(x > 0 && x <= INT16_MAX);
    return (item_id_t) x;
}

size_t
read_packet_id(struct pkt_buffer* r, mc_byte_t* id) {
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
    mc_word_t const len = read_word(r);
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
read_srv_pkt_0x11(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    if (!read_has(r, SRV_PKT_0x11_SIZE)) {
        r->overflow = true;
    }

    /* no overflow means this gets skipped */
    skip(r, SRV_PKT_0x11_SIZE);
    return 0;
}

size_t
read_srv_pkt_spawn_item(struct pkt_buffer* r, struct srv_pkt_spawn_item* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_SPAWN_ITEM_SIZE)) {
        r->overflow = true;
    }

    /* no overflow means this gets skipped */
    *pkt = (struct srv_pkt_spawn_item){
        .entity = read_entity_id(r),
        .item = read_item_id(r),
        .count = read_byte(r),
        .x = read_int(r),
        .y = read_int(r),
        .z = read_int(r),
        .yaw = read_byte(r),
        .pitch = read_byte(r),
        .unknown = read_byte(r),
    };
    return 0;
}

size_t read_srv_pkt_ent_pickup(struct pkt_buffer* r, struct srv_pkt_ent_pickup* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_PICKUP_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_PICKUP_SIZE;
    }

    *pkt = (struct srv_pkt_ent_pickup){
        .item = read_entity_id(r),
        .entity = read_entity_id(r),
    };
    return 0;
}

size_t read_srv_pkt_ent_destroy(struct pkt_buffer* r, struct srv_pkt_ent_destroy* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_DESTROY_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_DESTROY_SIZE;
    }

    *pkt = (struct srv_pkt_ent_destroy){
        .entity = read_entity_id(r),
    };
    return 0;
}

size_t
read_srv_pkt_ent_alive(struct pkt_buffer* r, struct srv_pkt_ent_alive* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_ALIVE_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_ALIVE_SIZE;
    }

    *pkt = (struct srv_pkt_ent_alive){
        .entity = read_entity_id(r),
    };
    return 0;
}

size_t
read_srv_pkt_0x1f(struct pkt_buffer* r, struct srv_pkt_0x1f* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_0x1F_SIZE)) {
        r->overflow = true;
        return SRV_PKT_0x1F_SIZE;
    }

    *pkt = (struct srv_pkt_0x1f){
        .id = read_entity_id(r),
        .unknown0 = read_byte(r),
        .unknown1 = read_byte(r),
        .unknown2 = read_byte(r),
    };
    return 0;
}

size_t
read_srv_pkt_ent_full_pos(struct pkt_buffer* r, struct srv_pkt_ent_full_pos* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_FULL_POS_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_FULL_POS_SIZE;
    }

    *pkt = (struct srv_pkt_ent_full_pos){
        .id = read_entity_id(r),
        .x = read_int(r),
        .y = read_int(r),
        .z = read_int(r),
        .yaw = read_byte(r),
        .pitch = read_byte(r),
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

    *pkt = (struct srv_pkt_chunk){
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
    mc_byte_t const x = read_byte(r);
    mc_byte_t const y = read_byte(r);
    mc_byte_t const z = read_byte(r);

    /* uncompressed data */
    mc_dword_t const compressed_size = read_dword(r);
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
read_srv_pkt_0x34(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    if (!read_has(r, SRV_PKT_0x34_MIN_SIZE)) {
        r->overflow = true;
    }

    skip(r, sizeof(mc_dword_t) * 2);
    mc_word_t const count = read_word(r);
    size_t const len = count * sizeof(mc_dword_t);

    /* some kind of variable data here */
    if (!read_has(r, len)) {
        r->overflow = true;
        return SRV_PKT_0x34_MIN_SIZE + len;
    }

    skip(r, len);
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
