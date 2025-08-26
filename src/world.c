#include "world.h"

#include <math.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera.h"

#define WORLD_GEN_FUNC(x, y) (8.0f * sin(0.125f * x) * sin(0.125f * y) + 10.0f)

static void _world_chunk_gen(struct Chunk *chunk) {
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

static int _world_create_open_table(sqlite3 *db) {
    int rc;
    char *err_msg = NULL;

    const char *sql =
        "CREATE TABLE IF NOT EXISTS chunks ("
        "x INTEGER NOT NULL,"
        "y INTEGER NOT NULL,"
        "z INTEGER NOT NULL,"
        "data BLOB,"
        "PRIMARY KEY (x, y, z));";

    if ((rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg)) != SQLITE_OK) {
        fprintf(stderr, "SQL error(%d): %s\n", rc, err_msg);
        sqlite3_free(err_msg);
        return 1;
    }
    printf("World table checked/created successfully.\n");
    return 0;
}

static int _world_load_chunk(sqlite3 *db, const int x, const int y, const int z,
                             struct Chunk *chunk) {
    const char *sql = "SELECT data FROM chunks WHERE x = ? AND y = ? AND z = ?;";
    int rc;
    sqlite3_stmt *stmt;

    chunk->modified = false;
    chunk->x = x;
    chunk->y = y;
    chunk->z = z;

    if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt, 1, chunk->x);
    sqlite3_bind_int(stmt, 2, chunk->y);
    sqlite3_bind_int(stmt, 3, chunk->z);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        // chunk not found
        sqlite3_finalize(stmt);
        _world_chunk_gen(chunk);
        printf("CHUNK[%d, %d, %d]: generated successfully.\n", chunk->x, chunk->y, chunk->z);
        return 0;
    }
    if (rc != SQLITE_ROW) {
        fprintf(stderr, "CHUNK[%d, %d, %d]: DB error loading chunk.\n", chunk->x, chunk->y,
                chunk->z);
        sqlite3_finalize(stmt);
        return 1;
    }

    const void *blob_data = sqlite3_column_blob(stmt, 0);
    int blob_size = sqlite3_column_bytes(stmt, 0);

    if (blob_data == NULL) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "CHUNK[%d, %d %d]: Null data in chunk. Regenerating\n", chunk->x, chunk->y,
                chunk->z);
        _world_chunk_gen(chunk);
        return 0;
    }

    if ((unsigned long)blob_size > CHUNK_BYTE_SIZE) {
        fprintf(stderr, "CHUNK[%d, %d %d]: blob_size greater than chunk buffer: %d > %lu.\n",
                chunk->x, chunk->y, chunk->z, blob_size, CHUNK_BYTE_SIZE);
        sqlite3_finalize(stmt);
        return 1;
    }

    memcpy(chunk->data, blob_data, blob_size);
    printf("CHUNK[%d, %d, %d]: loaded successfully.\n", chunk->x, chunk->y, chunk->z);
    sqlite3_finalize(stmt);

    return 0;
}

static int _world_save_chunk(sqlite3 *db, const struct Chunk *chunk) {
    if (!chunk->modified) {
        printf("CHUNK[%d, %d, %d]: not modified, skip saving.\n", chunk->x, chunk->y, chunk->z);
        return 0;
    }

    const char *sql = "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?);";
    int rc;
    sqlite3_stmt *stmt;

    if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement (%d): %s\n", rc, sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt, 1, chunk->x);
    sqlite3_bind_int(stmt, 2, chunk->y);
    sqlite3_bind_int(stmt, 3, chunk->z);
    sqlite3_bind_blob(stmt, 4, chunk->data, CHUNK_BYTE_SIZE, SQLITE_STATIC);

    if ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        fprintf(stderr, "CHUNK[%d, %d, %d]: DB error saving chunk.\n", chunk->x, chunk->y,
                chunk->z);
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    printf("CHUNK[%d, %d, %d]: saved successfully.\n", chunk->x, chunk->y, chunk->z);

    return 0;
}

static bool _chunk_in_render_dist(const struct Chunk *chunk, const int center_x, const int center_y,
                                  const int center_z) {
    return abs(chunk->x - center_x) <= RENDER_DISTANCE &&
           abs(chunk->y - center_y) <= RENDER_DISTANCE &&
           abs(chunk->z - center_z) <= RENDER_DISTANCE;
}

int load_world(sqlite3 *db, struct World *world, const struct Camera *cam) {
    int loaded_chunks = 0, errors = 0;

    world->db = db;

    world->center_x = cam->chunk_x;
    world->center_y = cam->chunk_y;
    world->center_z = cam->chunk_z;

    if (_world_create_open_table(world->db)) {
        fprintf(stderr, "Failed to open world table.\n");
        return 1;
    }

    world->chunks = malloc(sizeof(struct Chunk) * WORLD_VOLUME);
    if (world->chunks == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    for (int xi = 0, x = world->center_x - RENDER_DISTANCE; xi < LOADED_SIDE; xi++, x++) {
        for (int yi = 0, y = world->center_y - RENDER_DISTANCE; yi < LOADED_SIDE; yi++, y++) {
            for (int zi = 0, z = world->center_z - RENDER_DISTANCE; zi < LOADED_SIDE; zi++, z++) {
                int idx = zi * LOADED_SIDE * LOADED_SIDE + yi * LOADED_SIDE + xi;
                struct Chunk *chunk = &world->chunks[idx];
                if (_world_load_chunk(world->db, x, y, z, chunk))
                    errors++;
                else
                    loaded_chunks++;
            }
        }
    }

    printf("World loaded: %d chunks, %d errors\n", loaded_chunks, errors);

    return 0;
}

int close_world(struct World *world) {
    int saved_chunks = 0, errors = 0;

    if (world->chunks != NULL) {
        for (int x = 0; x < LOADED_SIDE; x++) {
            for (int y = 0; y < LOADED_SIDE; y++) {
                for (int z = 0; z < LOADED_SIDE; z++) {
                    int idx = z * LOADED_SIDE * LOADED_SIDE + y * LOADED_SIDE + x;
                    struct Chunk *chunk = &world->chunks[idx];
                    if (_world_save_chunk(world->db, chunk))
                        errors++;
                    else
                        saved_chunks++;
                    chunk->modified = false;
                }
            }
        }
        free(world->chunks);
    }

    printf("World saved: %d chunk, %d fatal errors.\n", saved_chunks, errors);

    return 0;
}

void update_world(struct World *world, struct Camera *cam) {
    if (cam->chunk_x == world->center_x && cam->chunk_y == world->center_y &&
        cam->chunk_z == world->center_z)
        return;

    int saved_chunks = 0, loaded_chunks = 0, errors = 0;

    printf("%d %d %d\n", world->center_x, world->center_y, world->center_z);

    int free_indices[WORLD_VOLUME];
    int free_indices_cnt = 0;

    for (int xi = 0, x = world->center_x - RENDER_DISTANCE; xi < LOADED_SIDE; xi++, x++) {
        for (int yi = 0, y = world->center_y - RENDER_DISTANCE; yi < LOADED_SIDE; yi++, y++) {
            for (int zi = 0, z = world->center_z - RENDER_DISTANCE; zi < LOADED_SIDE; zi++, z++) {
                int idx = zi * LOADED_SIDE * LOADED_SIDE + yi * LOADED_SIDE + xi;
                struct Chunk *chunk = &world->chunks[idx];

                if (!_chunk_in_render_dist(chunk, cam->chunk_x, cam->chunk_y, cam->chunk_z)) {
                    free_indices[free_indices_cnt++] = idx;
                    if (_world_save_chunk(world->db, chunk))
                        errors++;
                    else
                        saved_chunks++;
                }
            }
        }
    }

    if (free_indices_cnt > 0) {
        for (int xi = 0, x = cam->chunk_x - RENDER_DISTANCE; xi < LOADED_SIDE; xi++, x++) {
            for (int yi = 0, y = cam->chunk_y - RENDER_DISTANCE; yi < LOADED_SIDE; yi++, y++) {
                for (int zi = 0, z = cam->chunk_z - RENDER_DISTANCE; zi < LOADED_SIDE; zi++, z++) {
                    struct Chunk test_chunk = {.x = x, .y = y, .z = z};
                    if (_chunk_in_render_dist(&test_chunk, cam->chunk_x, cam->chunk_y,
                                              cam->chunk_z) &&
                        !_chunk_in_render_dist(&test_chunk, world->center_x, world->center_y,
                                               world->center_z)) {
                        struct Chunk *chunk = &world->chunks[free_indices[--free_indices_cnt]];
                        if (_world_load_chunk(world->db, x, y, z, chunk))
                            errors++;
                        else
                            loaded_chunks++;
                    }
                }
            }
        }
    }

    world->center_x = cam->chunk_x;
    world->center_y = cam->chunk_y;
    world->center_z = cam->chunk_z;

    printf("World update finished: %d loaded, %d saved, %d errors.\n", loaded_chunks, saved_chunks,
           errors);
}
