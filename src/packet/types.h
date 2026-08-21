/*
 * types.h: minecraft protocol types
 */

#ifndef OBSIDIAN_MC_TYPES_H
#define OBSIDIAN_MC_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mc_byte;
typedef uint16_t mc_word;
typedef uint32_t mc_dword;
typedef uint64_t mc_qword;
typedef int32_t mc_int;
typedef float mc_float;
typedef double mc_double;
typedef bool mc_bool;

#define SRV_PKT_HEARTBEAT_SIZE             0u
#define SRV_PKT_AUTH_SIZE                  8u
#define SRV_PKT_MESSAGE_MIN_SIZE           2u
#define SRV_PKT_FULL_POSITION_SIZE        41u
#define SRV_PKT_0x15_SIZE                 22u
#define SRV_PKT_ENTITY_SIZE                4u
#define SRV_PKT_CHUNK_SIZE                 9u
#define SRV_PKT_CHUNK_DATA_MIN_SIZE       17u
#define SRV_PKT_0x34_MIN_SIZE             10u
#define SRV_PKT_0x35_SIZE                 11u

enum srv_pkt {
    SRV_HEARTBEAT = 0x00,
    SRV_AUTH = 0x01,
    SRV_MESSAGE = 0x03,
    SRV_FULL_POSITION = 0x0d,
    SRV_0x15 = 0x15,
    SRV_ENTITY = 0x1e,
    SRV_CHUNK = 0x32,
    SRV_CHUNK_DATA = 0x33,
    SRV_0x34 = 0x34,
    SRV_0x35 = 0x35,
};

struct srv_pkt_auth {
    mc_dword unknown0;
    mc_dword unknown1;
};

struct srv_pkt_message {
    mc_word length;
    mc_byte const* bytes;
};

struct srv_pkt_full_position {
    mc_double x;
    mc_double head_y;
    mc_double y;
    mc_double z;
    mc_float rotation;
    mc_float head_pitch;
    mc_bool grounded;
};

struct srv_pkt_entity {
    mc_dword id;
};

struct srv_pkt_chunk {
    mc_int x;
    mc_int z;
    mc_bool load;
};

struct srv_pkt_chunk_data {
    mc_byte x;
    mc_byte y;
    mc_byte z;
    mc_dword compressed_size;
    mc_byte const* data;
};

#endif //OBSIDIAN_MC_TYPES_H
