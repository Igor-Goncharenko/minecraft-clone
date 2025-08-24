#ifndef WORLD_H
#define WORLD_H

#include <sqlite3.h>

#define LOADED_SIDE (10)
#define WORLD_VOLUME (LOADED_SIDE * LOADED_SIDE * LOADED_SIDE)

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

struct Chunk {
    int x, y, z;
    /* for now we have only one type of block */
    int data[CHUNK_VOLUME];
};

struct World {
    sqlite3 *db;
    struct Chunk *loaded_chunks;
};

int load_world(const char *filename, struct World *world);
int close_world(struct World *world);

#endif /* WORLD_H */
