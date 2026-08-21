/*
 * types.h: minecraft protocol types
 */

#ifndef OBSIDIAN_MC_TYPES_H
#define OBSIDIAN_MC_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mc_byte_t;
typedef uint16_t mc_word_t;
typedef uint32_t mc_dword_t;
typedef uint64_t mc_qword_t;
typedef int32_t mc_int_t;
typedef float mc_float_t;
typedef double mc_double_t;
typedef int mc_fixed27_5_t;
typedef bool mc_bool_t;

typedef mc_int_t entity_id_t;

#define SRV_PKT_HEARTBEAT_SIZE             0u
#define SRV_PKT_AUTH_SIZE                  8u
#define SRV_PKT_MESSAGE_MIN_SIZE           2u
#define SRV_PKT_FULL_POSITION_SIZE        41u
#define SRV_PKT_0x11_SIZE                  4u
#define SRV_PKT_0x15_SIZE                 22u
#define SRV_PKT_0x16_SIZE                  8u
#define SRV_PKT_ENT_DESTROY_SIZE           4u
#define SRV_PKT_0x1E_SIZE                  4u
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
    SRV_0x11 = 0x11,
    SRV_0x15 = 0x15,
    SRV_0x16 = 0x16,
    SRV_ENT_DESTROY = 0x1d,
    SRV_0x1E = 0x1e,
    SRV_0x1F = 0x1f,
    SRV_ENT_FULL_POS = 0x22,
    SRV_CHUNK = 0x32,
    SRV_CHUNK_DATA = 0x33,
    SRV_0x34 = 0x34,
    SRV_0x35 = 0x35,
};

struct srv_pkt_auth {
    mc_dword_t unknown0;
    mc_dword_t unknown1;
};

struct srv_pkt_message {
    mc_word_t length;
    mc_byte_t const* bytes;
};

struct srv_pkt_full_position {
    mc_double_t x;
    mc_double_t head_y;
    mc_double_t y;
    mc_double_t z;
    mc_float_t rotation;
    mc_float_t head_pitch;
    mc_bool_t grounded;
};

struct srv_pkt_0x16 {
    entity_id_t entity0;
    entity_id_t entity1;
};

struct srv_pkt_ent_destroy {
    entity_id_t entity;
};

struct srv_pkt_0x1e {
    entity_id_t entity;
};

struct srv_pkt_0x1f {
    entity_id_t id;
    mc_byte_t unknown0;
    mc_byte_t unknown1;
    mc_byte_t unknown2;
};

struct srv_pkt_ent_full_pos {
    entity_id_t id;
    mc_fixed27_5_t x;
    mc_fixed27_5_t y;
    mc_fixed27_5_t z;
    mc_byte_t yaw;
    mc_byte_t pitch;
};

struct srv_pkt_chunk {
    mc_int_t x;
    mc_int_t z;
    mc_bool_t load;
};

struct srv_pkt_chunk_data {
    mc_byte_t x;
    mc_byte_t y;
    mc_byte_t z;
    mc_dword_t compressed_size;
    mc_byte_t const* data;
};

#endif //OBSIDIAN_MC_TYPES_H
