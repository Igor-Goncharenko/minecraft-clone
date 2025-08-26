#include "chunk.h"

#include <math.h>

#define WORLD_GEN_FUNC(x, y) (8.0f * sin(0.125f * x) * sin(0.125f * y) + 10.0f)

void world_chunk_gen(struct Chunk *chunk) {
    chunk->modified = true;

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
}
