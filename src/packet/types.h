/*
 * types.h: minecraft protocol types
 */

#ifndef OBSIDIAN_MC_TYPES_H
#define OBSIDIAN_MC_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mc_byte;
typedef int8_t mc_i8;
typedef int16_t mc_i16;
typedef int32_t mc_i32;
typedef int64_t mc_i64;
typedef float mc_f32;
typedef double mc_f64;
typedef int32_t mc_f27_5;
typedef bool mc_bool;

typedef mc_i32 entity_id;

/*
 * world coordinates
 */
struct mc_w_coords {
    mc_f64 x;
    mc_f64 y;
    mc_f64 z;
};

/*
 * block coordinates
 */
struct mc_b_coords {
    mc_i32 x;
    mc_i16 y;
    mc_i32 z;
};

/*
 * chunk coordinates
 */
struct mc_c_coords {
    mc_i32 x;
    mc_i32 z;
};

/*
 * extents in block coordinates from an origin
 */
struct mc_extent {
    mc_i8 x;
    mc_i8 y;
    mc_i8 z;
};

/*
 * relative movement
 */
struct mc_offset {
    mc_i8 x;
    mc_i8 y;
    mc_i8 z;
};

#define SRV_PKT_HEARTBEAT_SIZE             0u
#define SRV_PKT_AUTH_SIZE                  8u
#define SRV_PKT_MESSAGE_MIN_SIZE           2u
#define SRV_PKT_FULL_POSITION_SIZE        41u
#define SRV_PKT_RECEIVE_ITEM_SIZE          4u
#define SRV_PKT_SPAWN_ITEM_SIZE           22u
#define SRV_PKT_ENT_PICKUP_SIZE            8u
#define SRV_PKT_ENT_DESTROY_SIZE           4u
#define SRV_PKT_ENT_ALIVE_SIZE             4u
#define SRV_PKT_0x1F_SIZE                  7u
#define SRV_PKT_ENT_FULL_POS_SIZE         18u
#define SRV_PKT_CHUNK_SIZE                 9u
#define SRV_PKT_CHUNK_DATA_MIN_SIZE       17u
#define SRV_PKT_0x34_MIN_SIZE             10u
#define SRV_PKT_0x35_SIZE                 11u

enum srv_pkt {
    SRV_HEARTBEAT = 0x00,
    SRV_AUTH = 0x01,
    SRV_MESSAGE = 0x03,
    SRV_FULL_POSITION = 0x0d,
    SRV_RECEIVE_ITEM = 0x11,
    SRV_SPAWN_ITEM = 0x15,
    SRV_ENT_PICKUP = 0x16,
    SRV_ENT_DESTROY = 0x1d,
    SRV_ENT_ALIVE = 0x1e,
    SRV_0x1F = 0x1f,
    SRV_ENT_FULL_POS = 0x22,
    SRV_CHUNK = 0x32,
    SRV_CHUNK_DATA = 0x33,
    SRV_0x34 = 0x34,
    SRV_0x35 = 0x35,
};

struct srv_pkt_auth {
    mc_i32 unknown0;
    mc_i32 unknown1;
};

struct srv_pkt_message {
    mc_i16 length;
    mc_byte const* bytes;
};

struct srv_pkt_full_position {
    mc_f64 x;
    mc_f64 head_y;
    mc_f64 y;
    mc_f64 z;
    mc_f32 rotation;
    mc_f32 head_pitch;
    mc_bool grounded;
};

struct srv_pkt_receive_item {
    mc_i16 item;
    mc_i8 count;
    mc_i16 durability;
};

struct srv_pkt_spawn_item {
    entity_id entity;
    mc_i16 item;
    mc_i8 count;
    mc_f27_5 x;
    mc_f27_5 y;
    mc_f27_5 z;
    mc_i8 yaw;
    mc_i8 pitch;
    mc_i8 unknown;
};

struct srv_pkt_ent_pickup {
    entity_id item;
    entity_id entity;
};

struct srv_pkt_ent_destroy {
    entity_id entity;
};

struct srv_pkt_ent_alive {
    entity_id entity;
};

struct srv_pkt_0x1f {
    entity_id id;
    mc_i8 unknown0;
    mc_i8 unknown1;
    mc_i8 unknown2;
};

struct srv_pkt_ent_full_pos {
    entity_id id;
    mc_f27_5 x;
    mc_f27_5 y;
    mc_f27_5 z;
    mc_i8 yaw;
    mc_i8 pitch;
};

struct srv_pkt_chunk {
    struct mc_c_coords chunk;
    mc_bool load;
};

struct srv_pkt_chunk_data {
    struct mc_b_coords origin;
    struct mc_extent extent;
    mc_i32 compressed_size;
    mc_byte const* data;
};

#endif //OBSIDIAN_MC_TYPES_H
