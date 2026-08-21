/*
 * buffer.c: packet buffer utility
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define write_head(r) (&r->data[r->cur])
#define write_avail(r) (r-> capacity - r->cur)

uint8_t*
pkt_buffer_init(struct pkt_buffer* r, size_t const sz) {
    assert(r != NULL);
    assert(sz > 0);

    uint8_t* buffer = malloc(sz);
    if (buffer == NULL) {
        return NULL;
    }

    *r = (struct pkt_buffer){
        .data = buffer,
        .capacity = sz,
        .overflow = false,
    };
    return buffer;
}

void
pkt_buffer_end(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);

    free(r->data);
    r->data = NULL;
}

size_t
pkt_buffer_drop(struct pkt_buffer* r) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(r->pos <= r->cur);
    assert(r->cur <= r->capacity);

    /* calculate how much to drop, and what to keep */
    size_t const drop = r->pos;
    size_t const keep = r->cur - r->pos;

    /* drop the head keep the tail */
    uint8_t const* tail = &r->data[drop];
    memmove(r->data, tail, keep);
    r->pos = 0;
    r->cur = keep;
    return drop;
}

uint8_t*
pkt_buffer_resize(struct pkt_buffer* r, size_t const sz) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(r->capacity < sz);

    /* allocate a new buffer */
    uint8_t* new = realloc(r->data, sz);
    if (new == NULL) {
        return NULL;
    }

    /* update the state */
    r->data = new;
    r->capacity = sz;
    return r->data;
}

size_t
pkt_buffer_fread(struct pkt_buffer* r, FILE* strm) {
    assert(r != NULL);
    assert(r->data != NULL);
    assert(r->cur <= r->capacity);
    assert(strm != NULL);

    /* read as many bytes as we can */
    uint8_t* buf = write_head(r);
    size_t const avail = write_avail(r);
    size_t const got = fread(buf, sizeof *buf, avail, strm);

    /* update our state */
    r->cur += got;
    return got;
}
