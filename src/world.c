#include "world.h"

#include <string.h>

void chunk_gen(struct Chunk *chunk) {
    memset(chunk, 0, sizeof(struct Chunk));

    // generate 16x16x2 plate in chunk
    for (int i = 0; i < 16 * 16 * 2; i++) {
        chunk->data[i] = 1;
    }
}
