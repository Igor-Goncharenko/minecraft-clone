#include "chunk.h"

#include <math.h>

#define WORLD_GEN_FUNC(x, y) (8.0f * sin(0.125f * x) * sin(0.125f * y) + 10.0f)

// clang-format off
static const float CUBE_VERTICES[] = {
    /* positions        * normals         */

    /* front */
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    /* right */
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    /* top */
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    /* back */
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    /* left */
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    /* bottom */
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
};

static const unsigned CUBE_INDICES[] = {
     0,  1,  2,     /* front */
     3,  2,  1,
     4,  5,  6,     /* right */
     7,  6,  5,
     8,  9, 10,     /* top */
    11, 10,  9,
    12, 13, 14,     /* back */
    15, 14, 13,
    16, 17, 18,     /* left */
    19, 18, 17,
    20, 21, 22,     /* bottom */
    23, 22, 21
};
// clang-format on

void chunk_init(struct Chunk *chunk, const int x, const int y, const int z) {
    chunk->modified = false;
    chunk->modified_unsaved = false;
    chunk->x = x;
    chunk->y = y;
    chunk->z = z;
    create_mesh(&chunk->mesh);
}

void chunk_destroy(struct Chunk *chunk) {
    delete_mesh(&chunk->mesh);
}

void chunk_gen(struct Chunk *chunk, const int x, const int y, const int z) {
    for (int ch_x = 0; ch_x < CHUNK_SIZE; ch_x++) {
        for (int ch_y = 0; ch_y < CHUNK_SIZE; ch_y++) {
            for (int ch_z = 0; ch_z < CHUNK_SIZE; ch_z++) {
                int idx = ch_z * CHUNK_SIZE * CHUNK_SIZE + ch_y * CHUNK_SIZE + ch_x;
                int x = chunk->x * CHUNK_SIZE + ch_x;
                int y = chunk->y * CHUNK_SIZE + ch_y;
                int z = chunk->z * CHUNK_SIZE + ch_z;
                chunk->data[idx] = (WORLD_GEN_FUNC(x, z) > y) ? 1 : 0;
            }
        }
    }

    chunk_init(chunk, x, y, z);
    chunk->modified_unsaved = true;
}

void chunk_mesh_update(struct Chunk *chunk) {
    chunk->modified = false;

    upload_mesh_data(&chunk->mesh, CUBE_VERTICES, sizeof(CUBE_VERTICES) / sizeof(float),
                     CUBE_INDICES, sizeof(CUBE_INDICES) / sizeof(unsigned));
}
