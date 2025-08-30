#ifndef WORLD_H
#define WORLD_H

#include <sqlite3.h>
#include <stdbool.h>

#include "camera.h"
#include "chunk.h"

#define RENDER_DISTANCE 10

#define LOADED_SIDE (RENDER_DISTANCE * 2 + 1)
#define WORLD_VOLUME (LOADED_SIDE * LOADED_SIDE * LOADED_SIDE)

#define WORLD_HASH_SIZE 4096

struct ChunkEntry {
    struct Chunk *chunk;
    struct ChunkEntry *next;
};

struct World {
    sqlite3 *db;
    struct Chunk *chunks;
    struct ChunkEntry *chunk_hash_table[WORLD_HASH_SIZE];

    int center_x;
    int center_y;
    int center_z;
};

int load_world(sqlite3 *db, struct World *world, const struct Camera *cam);
int close_world(struct World *world);
void update_world(struct World *world, struct Camera *cam);

#endif /* WORLD_H */
