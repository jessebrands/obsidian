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

    uint8_t const b = read_head(r)[0];
    r->pos += sizeof b;
    r->in_total += sizeof b;
    return b;
}

static mc_i8
read_i8(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(read_has(r, sizeof(mc_i8)));

    uint8_t const b = read_head(r)[0];
    r->pos += sizeof b;
    r->in_total += sizeof b;
    return (mc_i8) b;
}

static mc_i16
read_i16(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    uint16_t x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return (mc_i16) __builtin_bswap16(x);
}

static mc_i32
read_i32(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    uint32_t x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return (mc_i32) __builtin_bswap32(x);
}

static mc_i64
read_i64(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    uint64_t x;
    uint8_t const* b = read_head(r);

    /* copy and swap */
    assert(read_has(r, sizeof x));
    memcpy(&x, b, sizeof x);
    r->pos += sizeof x;
    r->in_total += sizeof x;
    return (mc_i64) __builtin_bswap64(x);
}

static mc_f32
read_f32(struct pkt_buffer* r) {
    mc_i32 const x = read_i32(r);
    mc_f32 f;

    /* coerce into float */
    memcpy(&f, &x, sizeof f);
    return f;
}

static mc_f64
read_f64(struct pkt_buffer* r) {
    mc_i64 const x = read_i64(r);
    mc_f64 d;

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

static entity_id
read_entity_id(struct pkt_buffer* r) {
    mc_i32 const x = read_i32(r);
    assert(x >= 0 && x <= INT32_MAX);
    return (entity_id) x;
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
        .unknown0 = read_i32(r),
        .unknown1 = read_i32(r),
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
    mc_i16 const len = read_i16(r);
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
        .x = read_f64(r),
        .head_y = read_f64(r),
        .y = read_f64(r),
        .z = read_f64(r),
        .rotation = read_f32(r),
        .head_pitch = read_f32(r),
        .grounded = read_bool(r),
    };
    return 0;
}

size_t
read_srv_pkt_receive_item(struct pkt_buffer* r, struct srv_pkt_receive_item* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_RECEIVE_ITEM_SIZE)) {
        r->overflow = true;
        return SRV_PKT_RECEIVE_ITEM_SIZE;
    }

    *pkt = (struct srv_pkt_receive_item){
        .item = read_i16(r),
        .count = read_i8(r),
        .durability = read_i16(r),
    };
    return 0;
}

size_t
read_srv_pkt_spawn_player(struct pkt_buffer* r, struct srv_pkt_spawn_player* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_SPAWN_PLAYER_MIN_SIZE)) {
        r->overflow = true;
        return SRV_PKT_SPAWN_PLAYER_MIN_SIZE;
    }

    entity_id const eid = read_entity_id(r);
    mc_i16 const name_length = read_i16(r);

    if (!read_has(r, name_length + 16)) {
        r->overflow = true;
        return SRV_PKT_SPAWN_PLAYER_MIN_SIZE;
    }

    *pkt = (struct srv_pkt_spawn_player){
        .entity = eid,
        .name_length = name_length,
        .name = read_bytes(r, name_length),
        .x = read_i32(r),
        .y = read_i32(r),
        .z = read_i32(r),
        .yaw = read_i8(r),
        .pitch = read_i8(r),
        .item = read_i16(r),
    };
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
        .item = read_i16(r),
        .count = read_i8(r),
        .x = read_i32(r),
        .y = read_i32(r),
        .z = read_i32(r),
        .yaw = read_i8(r),
        .pitch = read_i8(r),
        .unknown = read_i8(r),
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
read_srv_ent_move(struct pkt_buffer* r, struct srv_pkt_ent_move* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_MOVE_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_MOVE_SIZE;
    }

    *pkt = (struct srv_pkt_ent_move){
        .id = read_entity_id(r),
        .x = read_i8(r),
        .y = read_i8(r),
        .z = read_i8(r),
    };
    return 0;
}

size_t
read_srv_pkt_ent_look(struct pkt_buffer* r, struct srv_pkt_ent_look* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_LOOK_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_LOOK_SIZE;
    }

    *pkt = (struct srv_pkt_ent_look){
        .id = read_entity_id(r),
        .yaw = read_i8(r),
        .pitch = read_i8(r),
    };
    return 0;
}

size_t
read_srv_pkt_ent_move_look(struct pkt_buffer* r, struct srv_pkt_ent_move_look* pkt) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(pkt != NULL);

    if (!read_has(r, SRV_PKT_ENT_MOVE_LOOK_SIZE)) {
        r->overflow = true;
        return SRV_PKT_ENT_MOVE_LOOK_SIZE;
    }

    *pkt = (struct srv_pkt_ent_move_look){
        .id = read_entity_id(r),
        .x = read_i8(r),
        .y = read_i8(r),
        .z = read_i8(r),
        .yaw = read_i8(r),
        .pitch = read_i8(r),
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
        .x = read_i32(r),
        .y = read_i32(r),
        .z = read_i32(r),
        .yaw = read_i8(r),
        .pitch = read_i8(r),
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
        .chunk.x = read_i32(r),
        .chunk.z = read_i32(r),
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

    mc_i32 const origin_x = read_i32(r);
    mc_i16 const origin_y = read_i16(r);
    mc_i32 const origin_z = read_i32(r);

    /* read chunk extents */
    mc_i8 const x = read_i8(r);
    mc_i8 const y = read_i8(r);
    mc_i8 const z = read_i8(r);

    /* uncompressed data */
    mc_i32 const compressed_size = read_i32(r);
    if (read_avail(r) < compressed_size) {
        r->overflow = true;
        return SRV_PKT_CHUNK_DATA_MIN_SIZE + compressed_size;
    }

    *pkt = (struct srv_pkt_chunk_data){
        .origin.x = origin_x,
        .origin.y = origin_y,
        .origin.z = origin_z,
        .extent.x = x,
        .extent.y = y,
        .extent.z = z,
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

    skip(r, sizeof(mc_i32) * 2);
    mc_i16 const count = read_i16(r);
    size_t const len = count * sizeof(mc_i32);

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
