/*
 * reader_buffer.c: buffer operations
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"

uint8_t*
reader_init(struct pkt_reader* r, size_t const sz) {
    assert(r != NULL);
    assert(sz > 0);

    uint8_t* buffer = malloc(sz);
    if (buffer == NULL) {
        return NULL;
    }

    *r = (struct pkt_reader){
        .buffer = buffer,
        .pos = 0,
        .cur = 0,
        .capacity = sz,
        .offset = 0,
        .overflow = false,
    };
    return buffer;
}

void
reader_end(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);

    free(r->buffer);
    r->buffer = NULL;
}

size_t
reader_drop(struct pkt_reader* r) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(r->pos <= r->cur);
    assert(r->cur <= r->capacity);

    /* calculate how much to drop, and what to keep */
    size_t const drop = r->pos;
    size_t const keep = r->cur - r->pos;

    /* drop the head keep the tail */
    uint8_t const* tail = &r->buffer[drop];
    memmove(r->buffer, tail, keep);
    r->pos = 0;
    r->cur = keep;
    return drop;
}

uint8_t*
reader_grow(struct pkt_reader* r, size_t const sz) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(r->capacity < sz);

    /* allocate a new buffer */
    uint8_t* new = realloc(r->buffer, sz);
    if (new == NULL) {
        return NULL;
    }

    /* update the state */
    r->buffer = new;
    r->capacity = sz;
    return r->buffer;
}

size_t
reader_fill_from_file(struct pkt_reader* r, FILE* strm) {
    assert(r != NULL);
    assert(r->buffer != NULL);
    assert(r->cur <= r->capacity);
    assert(strm != NULL);

    /* read as many bytes as we can */
    uint8_t* buf = &r->buffer[r->cur];
    size_t const avail = r->capacity - r->cur;
    size_t const got = fread(buf, sizeof *buf, avail, strm);

    /* update our state */
    r->cur += got;
    return got;
}
