#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>

#include "mesh.h"

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_BYTE_SIZE (sizeof(short) * CHUNK_VOLUME)

enum CubeFace {
    FACE_FRONT = 0,
    FACE_RIGHT,
    FACE_TOP,
    FACE_BACK,
    FACE_LEFT,
    FACE_BOT,
};

struct Chunk {
    bool modified_unsaved, modified, mesh_created;
    int x, y, z;
    /* for now we have only one type of block */
    short data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    struct Mesh mesh;
};

void chunk_init(struct Chunk *chunk, const int x, const int y, const int z);

void chunk_destroy(struct Chunk *chunk);

void chunk_gen(struct Chunk *chunk);

void chunk_mesh_update(struct Chunk *chunk, const struct Chunk *nearby[6]);

#endif /* CHUNK_H */
