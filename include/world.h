#ifndef WORLD_H
#define WORLD_H

#include <sqlite3.h>
#include <stdbool.h>

#define LOADED_SIDE (10)
#define WORLD_VOLUME (LOADED_SIDE * LOADED_SIDE * LOADED_SIDE)

#define CHUNK_SIZE 16
#define CHUNK_VOLUME (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_BYTE_SIZE (sizeof(int) * CHUNK_VOLUME)

struct Chunk {
    bool modified;
    int x, y, z;
    /* for now we have only one type of block */
    int data[CHUNK_VOLUME];
};

struct World {
    sqlite3 *db;
    struct Chunk *loaded_chunks;
};

int load_world(sqlite3 *db, struct World *world);
int close_world(struct World *world);

#endif /* WORLD_H */
