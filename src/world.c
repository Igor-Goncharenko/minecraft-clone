#include "world.h"

#include <math.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORLD_GEN_FUNC(x, y) (8.0f * sin(0.125f * x) * sin(0.125f * y) + 10.0f)

void chunk_gen(struct Chunk *chunk) {
    memset(chunk->data, 0, sizeof(chunk->data));

    for (int ch_x = 0; ch_x < CHUNK_SIZE; ch_x++) {
        for (int ch_y = 0; ch_y < CHUNK_SIZE; ch_y++) {
            for (int ch_z = 0; ch_z < CHUNK_SIZE; ch_z++) {
                int idx = ch_z * CHUNK_SIZE * CHUNK_SIZE + ch_y * CHUNK_SIZE + ch_x;
                int x = chunk->x * CHUNK_SIZE + ch_x;
                int y = chunk->y * CHUNK_SIZE + ch_y;
                int z = chunk->z * CHUNK_SIZE + ch_z;
                (void)(x);
                (void)(y);
                chunk->data[idx] = (WORLD_GEN_FUNC(x, y) > z) ? 1 : 0;
            }
        }
    }

    printf("Chunk(x=%d, y=%d, z=%d) generated\n", chunk->x, chunk->y, chunk->z);
}

int db_open(const char *filename, sqlite3 **db) {
    char *err_msg = NULL;
    int rc = sqlite3_open(filename, db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to open database '%s'.\n", filename);
        return 1;
    }

    sqlite3_exec(*db, "PRAGMA journal_mode=WAL;", NULL, NULL, &err_msg);

    const char *sql =
        "CREATE TABLE IF NOT EXISTS chunks ("
        "x INTEGER NOT NULL,"
        "y INTEGER NOT NULL,"
        "z INTEGER NOT NULL,"
        "data BLOB,"
        "PRIMARY KEY (x, y, z));";

    rc = sqlite3_exec(*db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }
    printf("Table checked/created successfully.\n");
    return 0;
}

void db_close(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

int save_chunk(sqlite3 *db, const struct Chunk *chunk) {
    if (!db) return 1;

    const char *sql = "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    // Компилируем SQL-запрос в подготовленное выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt, 1, chunk->x);
    sqlite3_bind_int(stmt, 2, chunk->y);
    sqlite3_bind_int(stmt, 3, chunk->z);
    sqlite3_bind_blob(stmt, 4, chunk->data, CHUNK_VOLUME * sizeof(int), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    printf("Chunk [%d, %d, %d] saved.\n", chunk->x, chunk->y, chunk->z);
    return 0;
}

int load_chunk(sqlite3 *db, const int x, const int y, const int z, struct Chunk *chunk) {
    if (!db) return -1;

    chunk->x = x;
    chunk->y = y;
    chunk->z = z;

    const char *sql = "SELECT data FROM chunks WHERE x = ? AND y = ? AND z = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chunk->x);
    sqlite3_bind_int(stmt, 2, chunk->y);
    sqlite3_bind_int(stmt, 3, chunk->z);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const void *blob_data = sqlite3_column_blob(stmt, 0);
        int blob_size = sqlite3_column_bytes(stmt, 0);

        memcpy(chunk->data, blob_data, blob_size);
        sqlite3_finalize(stmt);
        printf("Chunk [%d, %d, %d] loaded.\n", chunk->x, chunk->y, chunk->z);
        return 0;
    } else if (rc == SQLITE_DONE) {
        // chunk not found
        sqlite3_finalize(stmt);
        chunk_gen(chunk);
        return 1;
    } else {
        // request error
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
}

int load_world(const char *filename, struct World *world) {
    db_open(filename, &world->db);

    world->loaded_chunks = malloc(sizeof(struct Chunk) * WORLD_VOLUME);

    for (int x = 0; x < LOADED_SIDE; x++) {
        for (int y = 0; y < LOADED_SIDE; y++) {
            for (int z = 0; z < LOADED_SIDE; z++) {
                int idx = z * LOADED_SIDE * LOADED_SIDE + y * LOADED_SIDE + x;
                load_chunk(world->db, x, y, z, &world->loaded_chunks[idx]);
            }
        }
    }

    printf("World loaded from '%s'\n", filename);

    return 0;
}

int close_world(struct World *world) {
    for (int x = 0; x < LOADED_SIDE; x++) {
        for (int y = 0; y < LOADED_SIDE; y++) {
            for (int z = 0; z < LOADED_SIDE; z++) {
                int idx = z * LOADED_SIDE * LOADED_SIDE + y * LOADED_SIDE + x;
                save_chunk(world->db, &world->loaded_chunks[idx]);
            }
        }
    }

    printf("World saved\n");

    db_close(world->db);

    return 0;
}
