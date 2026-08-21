/*
 * pkt_buffer.h: buffered packet reader/writer
 */

#ifndef OBSIDIAN_WRITER_H
#define OBSIDIAN_WRITER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "types.h"

/*
 * buffer that can be read from and written to
 */
struct pkt_buffer {
    uint8_t* data;
    size_t pos; /* read cursor */
    size_t cur; /* write cursor */
    size_t capacity;
    size_t in_total;
    size_t out_total;
    bool overflow; /* true if buffer is too small */
};

/*
 * initializes a packet buffer
 */
uint8_t*
pkt_buffer_init(struct pkt_buffer* r, size_t sz);

/*
 * releases all resources held by a packet buffer
 */
void
pkt_buffer_end(struct pkt_buffer* r);

/*
 * drops all data before the read cursor
 */
size_t
pkt_buffer_drop(struct pkt_buffer* r);

/*
 * resizes the packet buffer
 */
uint8_t*
pkt_buffer_resize(struct pkt_buffer* r, size_t sz);

/*
 * reads as many bytes as possible from a stream into the buffer
 */
size_t
pkt_buffer_fread(struct pkt_buffer* r, FILE* strm);

size_t
read_packet_id(struct pkt_buffer* r, mc_byte_t* id);

size_t
read_srv_pkt_auth(struct pkt_buffer* r, struct srv_pkt_auth* pkt);

size_t
read_srv_pkt_message(struct pkt_buffer* r, struct srv_pkt_message* pkt);

size_t
read_srv_pkt_full_position(struct pkt_buffer* r,
                           struct srv_pkt_full_position* pkt);

size_t
read_srv_pkt_0x11(struct pkt_buffer* r);

size_t
read_srv_pkt_0x15(struct pkt_buffer* r);

size_t
read_srv_pkt_0x16(struct pkt_buffer* r, struct srv_pkt_0x16* pkt);

size_t
read_srv_pkt_0x1d(struct pkt_buffer* r, struct srv_pkt_0x1d* pkt);

size_t
read_srv_pkt_0x1e(struct pkt_buffer* r, struct srv_pkt_0x1e* pkt);

size_t
read_srv_pkt_0x1f(struct pkt_buffer* r, struct srv_pkt_0x1f* pkt);

size_t
read_srv_pkt_ent_full_pos(struct pkt_buffer* r, struct srv_pkt_ent_full_pos* pkt);

size_t
read_srv_pkt_chunk(struct pkt_buffer* r, struct srv_pkt_chunk* pkt);

size_t
read_srv_pkt_chunk_data(struct pkt_buffer* r, struct srv_pkt_chunk_data* pkt);

size_t
read_srv_pkt_0x34(struct pkt_buffer* r);

size_t
read_srv_pkt_0x35(struct pkt_buffer* r);

#endif //OBSIDIAN_WRITER_H
