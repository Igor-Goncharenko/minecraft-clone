#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_BYTE_SIZE (sizeof(int) * CHUNK_VOLUME)

struct Chunk {
    bool modified;
    int x, y, z;
    /* for now we have only one type of block */
    int data[CHUNK_VOLUME];
};

void world_chunk_gen(struct Chunk *chunk);

#endif /* CHUNK_H */
