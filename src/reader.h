/*
 * reader.h: packet reader
 */

#ifndef OBSIDIAN_READER_H
#define OBSIDIAN_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "packet.h"

#define BUFFER_INITIAL_SIZE 128u

struct pkt_reader {
    uint8_t* buffer;
    size_t pos; /* read position */
    size_t cur; /* read cursor */
    size_t capacity;
    size_t offset; /* total file offset */
    bool overflow; /* true if buffer is too small */
};

uint8_t*
reader_init(struct pkt_reader* r, size_t sz);

void
reader_end(struct pkt_reader* r);

size_t
reader_drop(struct pkt_reader* r);

uint8_t*
reader_grow(struct pkt_reader* r, size_t sz);

size_t
reader_fill_from_file(struct pkt_reader* r, FILE* strm);

size_t
read_packet_id(struct pkt_reader* r, mc_byte* id);

size_t
read_srv_pkt_auth(struct pkt_reader* r, struct srv_pkt_auth* pkt);

size_t
read_srv_pkt_message(struct pkt_reader* r, struct srv_pkt_message* pkt);

size_t
read_srv_pkt_full_position(struct pkt_reader* r,
                           struct srv_pkt_full_position* pkt);

size_t
read_srv_pkt_0x15(struct pkt_reader* r);

size_t
read_srv_pkt_entity(struct pkt_reader* r, struct srv_pkt_entity* pkt);

size_t
read_srv_pkt_chunk(struct pkt_reader* r, struct srv_pkt_chunk* pkt);

size_t
read_srv_pkt_chunk_data(struct pkt_reader* r, struct srv_pkt_chunk_data* pkt);

size_t
read_srv_pkt_0x35(struct pkt_reader* r);

#endif //OBSIDIAN_READER_H
