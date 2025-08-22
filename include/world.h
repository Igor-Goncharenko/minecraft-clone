#ifndef WORLD_H
#define WORLD_H

#define CHUNK_SIZE 16

struct Chunk {
    /* for now we have only one type of block */
    int data[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
};

void chunk_gen(struct Chunk *chunk);

#endif /* WORLD_H */
