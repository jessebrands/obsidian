/*
 * types.h: minecraft protocol types
 */

#ifndef OBSIDIAN_MC_TYPES_H
#define OBSIDIAN_MC_TYPES_H

#include <stdbool.h>
#include <stdint.h>

enum animation {
    ANIMATION_ARM_SWING = 0x01,
};

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
#define SRV_PKT_ENT_HOLD_ITEM_SIZE         6u
#define SRV_PKT_RECEIVE_ITEM_SIZE          4u
#define SRV_PKT_ENT_ANIMATION_SIZE         5u
#define SRV_PKT_SPAWN_PLAYER_MIN_SIZE      6u
#define SRV_PKT_SPAWN_ITEM_SIZE           22u
#define SRV_PKT_ENT_PICKUP_SIZE            8u
#define SRV_PKT_ENT_DESTROY_SIZE           4u
#define SRV_PKT_ENT_ALIVE_SIZE             4u
#define SRV_PKT_ENT_MOVE_SIZE              7u
#define SRV_PKT_ENT_LOOK_SIZE              6u
#define SRV_PKT_ENT_MOVE_LOOK_SIZE         9u
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
    SRV_ENT_HOLD_ITEM = 0x10,
    SRV_RECEIVE_ITEM = 0x11,
    SRV_ENT_ANIMATION = 0x12,
    SRV_SPAWN_PLAYER = 0x14,
    SRV_SPAWN_ITEM = 0x15,
    SRV_ENT_PICKUP = 0x16,
    SRV_ENT_DESTROY = 0x1d,
    SRV_ENT_ALIVE = 0x1e,
    SRV_ENT_MOVE = 0x1f,
    SRV_ENT_LOOK = 0x20,
    SRV_ENT_MOVE_LOOK = 0x21,
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

struct srv_pkt_ent_hold_item {
    entity_id entity;
    mc_i16 item;
};

struct srv_pkt_receive_item {
    mc_i16 item;
    mc_i8 count;
    mc_i16 durability;
};

struct srv_pkt_ent_animation {
    entity_id entity;
    enum animation animation;
};

struct srv_pkt_spawn_player {
    entity_id entity;
    mc_i16 name_length;
    mc_byte const* name;
    mc_f27_5 x;
    mc_f27_5 y;
    mc_f27_5 z;
    mc_i8 yaw;
    mc_i8 pitch;
    mc_i16 item;
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

struct srv_pkt_ent_move {
    entity_id id;
    mc_i8 x;
    mc_i8 y;
    mc_i8 z;
};

struct srv_pkt_ent_look {
    entity_id id;
    mc_i8 yaw;
    mc_i8 pitch;
};

struct srv_pkt_ent_move_look {
    entity_id id;
    mc_i8 x;
    mc_i8 y;
    mc_i8 z;
    mc_i8 yaw;
    mc_i8 pitch;
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

char const*
animation_name(enum animation anim);

#endif //OBSIDIAN_MC_TYPES_H
