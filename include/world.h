#ifndef WORLD_H
#define WORLD_H

#include <sqlite3.h>
#include <stdbool.h>

#include "camera.h"

#define RENDER_DISTANCE 4

#define LOADED_SIDE (RENDER_DISTANCE * 2 + 1)
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

    int loaded_center_x;
    int loaded_center_y;
    int loaded_center_z;
};

int load_world(sqlite3 *db, struct World *world, const struct Camera *cam);
int close_world(struct World *world);
void update_world(struct World *world, struct Camera *cam);

#endif /* WORLD_H */
