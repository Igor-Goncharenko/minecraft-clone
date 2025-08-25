#include "world.h"

#include <math.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera.h"

#define WORLD_GEN_FUNC(x, y) (8.0f * sin(0.125f * x) * sin(0.125f * y) + 10.0f)

typedef enum {
    WORLD_OK = 0,
    CHUNK_NOT_FOUND,
    CHUNK_INVALID_DATA,
    CHUNK_MEMORY_ERROR,

    DB_ERROR,
} world_error_e;

static void _world_chunk_gen(struct Chunk *chunk) {
    chunk->modified = true;

    for (int ch_x = 0; ch_x < CHUNK_SIZE; ch_x++) {
        for (int ch_y = 0; ch_y < CHUNK_SIZE; ch_y++) {
            for (int ch_z = 0; ch_z < CHUNK_SIZE; ch_z++) {
                int idx = ch_z * CHUNK_SIZE * CHUNK_SIZE + ch_y * CHUNK_SIZE + ch_x;
                int x = chunk->x * CHUNK_SIZE + ch_x;
                int y = chunk->y * CHUNK_SIZE + ch_y;
                int z = chunk->z * CHUNK_SIZE + ch_z;
                chunk->data[idx] = (WORLD_GEN_FUNC(x, y) > z) ? 1 : 0;
            }
        }
    }
}

static world_error_e _world_create_open_table(sqlite3 *db) {
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
        return DB_ERROR;
    }
    printf("World table checked/created successfully.\n");
    return WORLD_OK;
}

static world_error_e _world_save_chunk(sqlite3 *db, const struct Chunk *chunk) {
    const char *sql = "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?);";
    int rc;
    sqlite3_stmt *stmt;

    if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement (%d): %s\n", rc, sqlite3_errmsg(db));
        return DB_ERROR;
    }

    sqlite3_bind_int(stmt, 1, chunk->x);
    sqlite3_bind_int(stmt, 2, chunk->y);
    sqlite3_bind_int(stmt, 3, chunk->z);
    sqlite3_bind_blob(stmt, 4, chunk->data, CHUNK_BYTE_SIZE, SQLITE_STATIC);

    if ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return DB_ERROR;
    }

    sqlite3_finalize(stmt);
    return WORLD_OK;
}

static world_error_e _world_load_chunk(sqlite3 *db, const int x, const int y, const int z,
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
        return DB_ERROR;
    }

    sqlite3_bind_int(stmt, 1, chunk->x);
    sqlite3_bind_int(stmt, 2, chunk->y);
    sqlite3_bind_int(stmt, 3, chunk->z);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        // chunk not found
        sqlite3_finalize(stmt);
        return CHUNK_NOT_FOUND;
    }

    if (rc != SQLITE_ROW) {
        // db error
        sqlite3_finalize(stmt);
        return DB_ERROR;
    }

    const void *blob_data = sqlite3_column_blob(stmt, 0);
    int blob_size = sqlite3_column_bytes(stmt, 0);

    if (blob_data == NULL) {
        fprintf(stderr, "CHUNK[%d, %d %d]: Null data in chunk.\n", chunk->x, chunk->y, chunk->z);
        sqlite3_finalize(stmt);
        return CHUNK_INVALID_DATA;
    }

    if ((unsigned long)blob_size > CHUNK_BYTE_SIZE) {
        fprintf(stderr, "CHUNK[%d, %d %d]: blob_size greater than chunk buffer: %d > %lu.\n",
                chunk->x, chunk->y, chunk->z, blob_size, CHUNK_BYTE_SIZE);
        sqlite3_finalize(stmt);
        return CHUNK_MEMORY_ERROR;
    }

    memcpy(chunk->data, blob_data, blob_size);

    sqlite3_finalize(stmt);
    return WORLD_OK;
}

int load_world(sqlite3 *db, struct World *world, const struct Camera *cam) {
    world_error_e err;
    int loaded_chunks = 0, errors = 0, fatal_errors = 0;

    world->db = db;

    world->loaded_center_x = cam->chunk_x;
    world->loaded_center_y = cam->chunk_y;
    world->loaded_center_z = cam->chunk_z;

    if (_world_create_open_table(world->db) != WORLD_OK) {
        fprintf(stderr, "Failed to open world table.\n");
        return 1;
    }

    world->loaded_chunks = malloc(sizeof(struct Chunk) * WORLD_VOLUME);
    if (world->loaded_chunks == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    int x = world->loaded_center_x - RENDER_DISTANCE;
    for (int xi = 0; xi < LOADED_SIDE; xi++, x++) {
        int y = world->loaded_center_y - RENDER_DISTANCE;
        for (int yi = 0; yi < LOADED_SIDE; yi++, y++) {
            int z = world->loaded_center_z - RENDER_DISTANCE;
            for (int zi = 0; zi < LOADED_SIDE; zi++, z++) {
                int idx = zi * LOADED_SIDE * LOADED_SIDE + yi * LOADED_SIDE + xi;
                struct Chunk *chunk = &world->loaded_chunks[idx];
                err = _world_load_chunk(world->db, x, y, z, chunk);

                switch (err) {
                    case WORLD_OK:
                        printf("CHUNK[%d, %d, %d]: loaded successfully.\n", chunk->x, chunk->y,
                               chunk->z);
                        loaded_chunks++;
                        break;
                    case CHUNK_NOT_FOUND:
                        _world_chunk_gen(chunk);
                        printf("CHUNK[%d, %d, %d]: generated successfully.\n", chunk->x, chunk->y,
                               chunk->z);
                        loaded_chunks++;
                        break;
                    case CHUNK_INVALID_DATA:
                        fprintf(stderr, "CHUNK[%d, %d, %d]: regenerating corrupted chunk.\n",
                                chunk->x, chunk->y, chunk->z);
                        _world_chunk_gen(chunk);
                        errors++;
                        break;
                    case DB_ERROR:
                        fprintf(stderr, "CHUNK[%d, %d, %d]: DB error loading chunk.\n", chunk->x,
                                chunk->y, chunk->z);
                        fatal_errors++;
                        break;
                    case CHUNK_MEMORY_ERROR:
                        fprintf(stderr, "CHUNK[%d, %d, %d]: Memory error loading chunk.\n",
                                chunk->x, chunk->y, chunk->z);
                        fatal_errors++;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    printf("World loaded from: %d chunks, %d errors, %d fatal errors.\n", loaded_chunks, errors,
           fatal_errors);

    return 0;
}

int close_world(struct World *world) {
    world_error_e err;
    int saved_chunks = 0, fatal_errors = 0;

    if (world->loaded_chunks != NULL) {
        for (int x = 0; x < LOADED_SIDE; x++) {
            for (int y = 0; y < LOADED_SIDE; y++) {
                for (int z = 0; z < LOADED_SIDE; z++) {
                    int idx = z * LOADED_SIDE * LOADED_SIDE + y * LOADED_SIDE + x;
                    struct Chunk *chunk = &world->loaded_chunks[idx];
                    if (!chunk->modified) {
                        printf("CHUNK[%d, %d, %d]: not modified, skip saving.\n", chunk->x,
                               chunk->y, chunk->z);
                        continue;
                    }

                    err = _world_save_chunk(world->db, chunk);

                    switch (err) {
                        case WORLD_OK:
                            printf("CHUNK[%d, %d, %d]: saved successfully.\n", chunk->x, chunk->y,
                                   chunk->z);
                            saved_chunks++;
                            break;
                        case DB_ERROR:
                            fprintf(stderr, "CHUNK[%d, %d, %d]: DB error saving chunk.\n", chunk->x,
                                    chunk->y, chunk->z);
                            fatal_errors++;
                            break;
                        default:
                            break;
                    }
                    chunk->modified = false;
                }
            }
        }
        free(world->loaded_chunks);
    }

    printf("World saved: %d chunk, %d fatal errors.\n", saved_chunks, fatal_errors);

    return 0;
}

static bool _check_chunk_in_render_dist(const struct Chunk *chunk, const int x, const int y,
                                        const int z) {
    return abs(chunk->x - x) <= RENDER_DISTANCE && abs(chunk->y - y) <= RENDER_DISTANCE &&
           abs(chunk->z - z) <= RENDER_DISTANCE;
}

void update_world(struct World *world, struct Camera *cam) {
    if (cam->chunk_x == world->loaded_center_x && cam->chunk_y == world->loaded_center_y &&
        cam->chunk_z == world->loaded_center_z)
        return;

    printf("%d %d %d\n", world->loaded_center_x, world->loaded_center_y, world->loaded_center_z);

    int free_indices[WORLD_VOLUME];
    int free_indices_cnt = 0;

    int x = world->loaded_center_x - RENDER_DISTANCE;
    for (int xi = 0; xi < LOADED_SIDE; xi++, x++) {
        int y = world->loaded_center_y - RENDER_DISTANCE;
        for (int yi = 0; yi < LOADED_SIDE; yi++, y++) {
            int z = world->loaded_center_z - RENDER_DISTANCE;
            for (int zi = 0; zi < LOADED_SIDE; zi++, z++) {
                int idx = zi * LOADED_SIDE * LOADED_SIDE + yi * LOADED_SIDE + xi;
                struct Chunk *chunk = &world->loaded_chunks[idx];

                if (!_check_chunk_in_render_dist(chunk, cam->chunk_x, cam->chunk_y, cam->chunk_z)) {
                    free_indices[free_indices_cnt++] = idx;
                    switch (_world_save_chunk(world->db, chunk)) {
                        case WORLD_OK:
                            printf("CHUNK[%d, %d, %d]: saved successfully.\n", chunk->x, chunk->y,
                                   chunk->z);
                            break;
                        case DB_ERROR:
                            fprintf(stderr, "CHUNK[%d, %d, %d]: DB error saving chunk.\n", chunk->x,
                                    chunk->y, chunk->z);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    for (int i = 0; i < free_indices_cnt; i++) {
        printf("%d ", free_indices[i]);
    }
    printf("\n");

    x = cam->chunk_x - RENDER_DISTANCE;
    for (int xi = 0; xi < LOADED_SIDE; xi++, x++) {
        int y = cam->chunk_y - RENDER_DISTANCE;
        for (int yi = 0; yi < LOADED_SIDE; yi++, y++) {
            int z = cam->chunk_z - RENDER_DISTANCE;
            for (int zi = 0; zi < LOADED_SIDE; zi++, z++) {
                struct Chunk test_chunk = {.x = x, .y = y, .z = z};
                if (_check_chunk_in_render_dist(&test_chunk, cam->chunk_x, cam->chunk_y,
                                                cam->chunk_z) &&
                    !_check_chunk_in_render_dist(&test_chunk, world->loaded_center_x,
                                                 world->loaded_center_y, world->loaded_center_z)) {
                    struct Chunk *chunk = &world->loaded_chunks[free_indices[--free_indices_cnt]];
                    switch (_world_load_chunk(world->db, x, y, z, chunk)) {
                        case WORLD_OK:
                            printf("CHUNK[%d, %d, %d]: loaded successfully.\n", chunk->x, chunk->y,
                                   chunk->z);
                            break;
                        case CHUNK_NOT_FOUND:
                            _world_chunk_gen(chunk);
                            printf("CHUNK[%d, %d, %d]: generated successfully.\n", chunk->x,
                                   chunk->y, chunk->z);
                            break;
                        case CHUNK_INVALID_DATA:
                            fprintf(stderr, "CHUNK[%d, %d, %d]: regenerating corrupted chunk.\n",
                                    chunk->x, chunk->y, chunk->z);
                            _world_chunk_gen(chunk);
                            break;
                        case DB_ERROR:
                            fprintf(stderr, "CHUNK[%d, %d, %d]: DB error loading chunk.\n",
                                    chunk->x, chunk->y, chunk->z);
                            break;
                        case CHUNK_MEMORY_ERROR:
                            fprintf(stderr, "CHUNK[%d, %d, %d]: Memory error loading chunk.\n",
                                    chunk->x, chunk->y, chunk->z);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    world->loaded_center_x = cam->chunk_x;
    world->loaded_center_y = cam->chunk_y;
    world->loaded_center_z = cam->chunk_z;
}
