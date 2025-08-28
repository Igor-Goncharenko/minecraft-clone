#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>

#include "mesh.h"

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_BYTE_SIZE (sizeof(int) * CHUNK_VOLUME)

struct Chunk {
    bool modified_unsaved, modified;
    int x, y, z;
    /* for now we have only one type of block */
    int data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    struct Mesh mesh;
};

void chunk_init(struct Chunk *chunk, const int x, const int y, const int z);

void chunk_destroy(struct Chunk *chunk);

void chunk_gen(struct Chunk *chunk, const int x, const int y, const int z);

void chunk_mesh_update(struct Chunk *chunk);

#endif /* CHUNK_H */
