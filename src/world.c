#include "world.h"

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "camera.h"
#include "chunk.h"
#include "utils.h"

static unsigned _hash_chunk_coords(const int x, const int y, const int z) {
    return (unsigned)((x * 73856093) ^ (y * 19349663) ^ (z * 83492791)) % WORLD_HASH_SIZE;
}

static void _hash_table_init(struct World *world) {
    for (int i = 0; i < WORLD_HASH_SIZE; i++) {
        world->chunk_hash_table[i] = NULL;
    }
}

static void _hash_table_destroy(struct World *world) {
    for (int i = 0; i < WORLD_HASH_SIZE; i++) {
        struct ChunkEntry *entry = world->chunk_hash_table[i];
        while (entry != NULL) {
            struct ChunkEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }
}

static void _hash_table_add(struct World *world, struct Chunk *chunk) {
    unsigned hash = _hash_chunk_coords(chunk->x, chunk->y, chunk->z);
    struct ChunkEntry *entry = malloc(sizeof(struct ChunkEntry));
    entry->chunk = chunk;
    entry->next = world->chunk_hash_table[hash];
    world->chunk_hash_table[hash] = entry;
}

static struct Chunk *_hash_table_find(const struct World *world, const int x, const int y,
                                      const int z) {
    unsigned hash = _hash_chunk_coords(x, y, z);

    for (struct ChunkEntry *entry = world->chunk_hash_table[hash]; entry; entry = entry->next) {
        if (entry->chunk->x == x && entry->chunk->y == y && entry->chunk->z == z) {
            return entry->chunk;
        }
    }
    return NULL;
}

// static void _hash_table_remove(struct World *world, const int x, const int y, const int z) {
//     unsigned hash = _hash_chunk_coords(x, y, z);
//     struct ChunkEntry **prev = &world->chunk_hash_table[hash];
//
//     for (struct ChunkEntry *entry = world->chunk_hash_table[hash]; entry; entry = entry->next) {
//         if (entry->chunk->x == x && entry->chunk->y == y && entry->chunk->z == z) {
//             *prev = entry->next;
//             free(entry);
//             return;
//         }
//         prev = &entry->next;
//     }
// }

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

static int _save_chunks_batch(struct World *world, struct Chunk **chunks, const int count) {
    int rc;
    int errors = 0;

    sqlite3_exec(world->db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    for (int i = 0; i < count; i++) {
        size_t compressed_size;
        uint8_t *compressed;
        rc = compress_chunk_data((uint8_t *)chunks[i]->data, CHUNK_BYTE_SIZE, &compressed,
                                 &compressed_size);

        if (rc == Z_OK) {
            sqlite3_bind_blob(world->save_stmt, 4, compressed, compressed_size, SQLITE_STATIC);
        }
        // else {
        //     sqlite3_bind_blob(world->save_stmt, 4, chunks[i]->data, CHUNK_BYTE_SIZE,
        //     SQLITE_STATIC);
        // }

        sqlite3_bind_int(world->save_stmt, 1, chunks[i]->x);
        sqlite3_bind_int(world->save_stmt, 2, chunks[i]->y);
        sqlite3_bind_int(world->save_stmt, 3, chunks[i]->z);

        if ((rc = sqlite3_step(world->save_stmt)) != SQLITE_DONE) {
            fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(world->db));
            fprintf(stderr, "CHUNK[%d, %d, %d]: DB error saving chunk.\n", chunks[i]->x,
                    chunks[i]->y, chunks[i]->z);
            errors++;
        }

        sqlite3_reset(world->save_stmt);
        sqlite3_clear_bindings(world->save_stmt);
    }

    sqlite3_exec(world->db, "COMMIT;", NULL, NULL, NULL);

    for (int i = 0; i < count; i++) chunk_destroy(chunks[i]);

    return errors;
}

static int _load_chunk_batch(struct World *world, const int *coords, const int count,
                             struct Chunk **dest) {
    int rc;

    for (int i = 0; i < count; i++) {
        const int x = coords[i * 3 + 0];
        const int y = coords[i * 3 + 1];
        const int z = coords[i * 3 + 2];

        chunk_init(dest[i], x, y, z);

        sqlite3_bind_int(world->load_stmt, 1, x);
        sqlite3_bind_int(world->load_stmt, 2, y);
        sqlite3_bind_int(world->load_stmt, 3, z);

        rc = sqlite3_step(world->load_stmt);
        if (rc == SQLITE_ROW) {
            const void *blob_data = sqlite3_column_blob(world->load_stmt, 0);
            const size_t blob_size = sqlite3_column_bytes(world->load_stmt, 0);

            uint8_t *decompressed;
            rc = decompress_chunk_data(blob_data, blob_size, &decompressed, CHUNK_BYTE_SIZE);

            if (rc != Z_OK) {
                fprintf(stderr, "CHUNK[%d, %d, %d]: Failed to decompress data. Regenerating\n", x,
                        y, z);
                chunk_gen(dest[i]);
            } else if (blob_data != NULL && blob_size <= CHUNK_BYTE_SIZE) {
                memcpy(dest[i]->data, decompressed, CHUNK_BYTE_SIZE);
            } else {
                fprintf(stderr, "CHUNK[%d, %d %d]: blob error. Regenerating\n", x, y, z);
                chunk_gen(dest[i]);
            }

            if (decompressed != NULL) free(decompressed);
        } else if (rc == SQLITE_DONE) {
            chunk_gen(dest[i]);
        } else {
            fprintf(stderr, "CHUNK[%d, %d, %d]: DB error loading chunk.\n", x, y, z);
            chunk_gen(dest[i]);
        }

        sqlite3_reset(world->load_stmt);
        sqlite3_clear_bindings(world->load_stmt);
    }

    return 0;
}

static bool _chunk_in_render_dist(const struct Chunk *chunk, const int center_x, const int center_y,
                                  const int center_z) {
    return abs(chunk->x - center_x) <= RENDER_DISTANCE &&
           abs(chunk->y - center_y) <= RENDER_DISTANCE &&
           abs(chunk->z - center_z) <= RENDER_DISTANCE;
}

static int _unload_chunk_out_of_range(struct World *world, struct Camera *cam, int *free_indices) {
    int free_indices_cnt = 0;
    struct Chunk *to_save[WORLD_VOLUME];
    int count = 0;

    for (int i = 0; i < WORLD_HASH_SIZE; i++) {
        struct ChunkEntry *entry = world->chunk_hash_table[i];
        struct ChunkEntry *prev = NULL;
        while (entry) {
            struct Chunk *chunk = entry->chunk;

            if (!_chunk_in_render_dist(chunk, cam->chunk_x, cam->chunk_y, cam->chunk_z)) {
                //_world_save_chunk(world->db, chunk);
                to_save[count++] = chunk;
                chunk_destroy(chunk);
                free_indices[free_indices_cnt++] = chunk - world->chunks;

                if (prev) {
                    prev->next = entry->next;
                } else {
                    world->chunk_hash_table[i] = entry->next;
                }

                struct ChunkEntry *to_free = entry;
                entry = entry->next;
                free(to_free);
            } else {
                prev = entry;
                entry = entry->next;
            }
        }
    }

    _save_chunks_batch(world, to_save, count);

    return free_indices_cnt;
}

static void _load_new_chunks_in_range(struct World *world, struct Camera *cam,
                                      const int *free_indices, const int free_indices_cnt) {
    int chunks_loaded = 0;

    struct Chunk *chunks_to_load[WORLD_VOLUME];
    int coords[WORLD_VOLUME * 3];
    int count = 0;

    for (int dx = -RENDER_DISTANCE; dx <= RENDER_DISTANCE; dx++) {
        for (int dy = -RENDER_DISTANCE; dy <= RENDER_DISTANCE; dy++) {
            for (int dz = -RENDER_DISTANCE; dz <= RENDER_DISTANCE; dz++) {
                int x = cam->chunk_x + dx;
                int y = cam->chunk_y + dy;
                int z = cam->chunk_z + dz;

                if (chunks_loaded < free_indices_cnt && _hash_table_find(world, x, y, z) == NULL) {
                    int idx = free_indices[chunks_loaded++];
                    struct Chunk *chunk = &world->chunks[idx];

                    coords[count * 3 + 0] = x;
                    coords[count * 3 + 1] = y;
                    coords[count * 3 + 2] = z;
                    chunks_to_load[count] = chunk;
                    count++;
                }
            }
        }
    }

    _load_chunk_batch(world, coords, count, chunks_to_load);

    for (int i = 0; i < count; i++) {
        _hash_table_add(world, chunks_to_load[i]);
    }
}

static void _world_update_chunk_meshes(struct World *world) {
    for (int xi = 0; xi < LOADED_SIDE; xi++) {
        for (int yi = 0; yi < LOADED_SIDE; yi++) {
            for (int zi = 0; zi < LOADED_SIDE; zi++) {
                const int idx = zi * LOADED_SIDE * LOADED_SIDE + yi * LOADED_SIDE + xi;
                struct Chunk *chunk = &world->chunks[idx];

                if (chunk->mesh_created) continue;

                const struct Chunk *nearby_chunks[6] = {
                    [FACE_FRONT] = _hash_table_find(world, chunk->x, chunk->y - 1, chunk->z),
                    [FACE_RIGHT] = _hash_table_find(world, chunk->x + 1, chunk->y, chunk->z),
                    [FACE_TOP] = _hash_table_find(world, chunk->x, chunk->y, chunk->z + 1),
                    [FACE_BACK] = _hash_table_find(world, chunk->x, chunk->y + 1, chunk->z),
                    [FACE_LEFT] = _hash_table_find(world, chunk->x - 1, chunk->y, chunk->z),
                    [FACE_BOT] = _hash_table_find(world, chunk->x, chunk->y, chunk->z - 1),
                };

                chunk_mesh_update(chunk, nearby_chunks);
            }
        }
    }
}

int load_world(sqlite3 *db, struct World *world, const struct Camera *cam) {
    const char *save_sql = "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?)";
    const char *load_sql = "SELECT data FROM chunks WHERE x=? AND y=? AND z=?";

    int rc;
    int errors = 0;

    memset(world, 0, sizeof(struct World));

    world->db = db;

    world->center_x = cam->chunk_x;
    world->center_y = cam->chunk_y;
    world->center_z = cam->chunk_z;

    if (_world_create_open_table(world->db)) {
        fprintf(stderr, "Failed to open world table.\n");
        return 1;
    }
    if ((rc = sqlite3_prepare_v2(db, save_sql, -1, &world->save_stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement (%d): %s\n", rc, sqlite3_errmsg(db));
        return 1;
    }
    if ((rc = sqlite3_prepare_v2(db, load_sql, -1, &world->load_stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement (%d): %s\n", rc, sqlite3_errmsg(db));
        return 1;
    }

    _hash_table_init(world);

    struct Chunk *chunks_to_load[WORLD_VOLUME];
    int coords[WORLD_VOLUME * 3];
    int count = 0;

    world->chunks = malloc(sizeof(struct Chunk) * WORLD_VOLUME);
    if (world->chunks == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    int x = world->center_x - RENDER_DISTANCE;
    for (int xi = 0; xi < LOADED_SIDE; xi++, x++) {
        int y = world->center_y - RENDER_DISTANCE;

        for (int yi = 0; yi < LOADED_SIDE; yi++, y++) {
            int z = world->center_z - RENDER_DISTANCE;

            for (int zi = 0; zi < LOADED_SIDE; zi++, z++) {
                int idx = zi * LOADED_SIDE * LOADED_SIDE + yi * LOADED_SIDE + xi;
                struct Chunk *chunk = &world->chunks[idx];

                coords[count * 3 + 0] = x;
                coords[count * 3 + 1] = y;
                coords[count * 3 + 2] = z;
                chunks_to_load[count] = chunk;
                count++;
            }
        }
    }

    _load_chunk_batch(world, coords, count, chunks_to_load);

    for (int i = 0; i < count; i++) {
        _hash_table_add(world, chunks_to_load[i]);
    }

    _world_update_chunk_meshes(world);

    printf("World loaded: %d errors\n", errors);

    return 0;
}

int close_world(struct World *world) {
    int errors = 0;

    struct Chunk *to_save[WORLD_VOLUME];

    if (world->chunks != NULL) {
        for (int i = 0; i < WORLD_VOLUME; i++) {
            struct Chunk *chunk = &world->chunks[i];
            to_save[i] = chunk;
        }

        errors = _save_chunks_batch(world, to_save, WORLD_VOLUME);

        free(world->chunks);
    }

    _hash_table_destroy(world);

    if (world->save_stmt != NULL) sqlite3_finalize(world->save_stmt);
    if (world->load_stmt != NULL) sqlite3_finalize(world->load_stmt);

    printf("World saved: %d errors.\n", errors);

    return 0;
}

void update_world(struct World *world, struct Camera *cam) {
    if (cam->chunk_x == world->center_x && cam->chunk_y == world->center_y &&
        cam->chunk_z == world->center_z)
        return;

    int free_indices[WORLD_VOLUME];
    int free_indices_cnt = 0;

    free_indices_cnt = _unload_chunk_out_of_range(world, cam, free_indices);

    if (free_indices_cnt > 0) {
        _load_new_chunks_in_range(world, cam, free_indices, free_indices_cnt);
    }

    _world_update_chunk_meshes(world);

    world->center_x = cam->chunk_x;
    world->center_y = cam->chunk_y;
    world->center_z = cam->chunk_z;
}
