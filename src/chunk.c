#include "chunk.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WORLD_GEN_FUNC(x, y) (8.0f * sin(0.125f * x) * sin(0.125f * y) + 10.0f)

/*
 * +x : right
 * -x : left
 * +y : back
 * -y : front
 * +z : top
 * -z : bot
 */

// clang-format off
static const float CUBE_VERTICES[] = {
    /* positions        * normals         */

    /* front */
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    /* right */
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    /* top */
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    /* back */
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    /* left */
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    /* bottom */
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
};
// clang-format on

static void _append_face_vert_to_arr(float *vertices, unsigned *n_vertices, unsigned *indices,
                                     unsigned *n_indices, const int x, const int y, const int z,
                                     const enum CubeFace face) {
    const int vert_offset = (int)face * 24;
    const unsigned base_n_vertex = *n_vertices / 6;

    for (int i = 0; i < 4; i++) {
        const int src_idx = vert_offset + i * 6;
        const int dst_idx = *n_vertices + i * 6;

        vertices[dst_idx + 0] = CUBE_VERTICES[src_idx + 0] + (float)x;
        vertices[dst_idx + 1] = CUBE_VERTICES[src_idx + 1] + (float)y;
        vertices[dst_idx + 2] = CUBE_VERTICES[src_idx + 2] + (float)z;

        vertices[dst_idx + 3] = CUBE_VERTICES[src_idx + 3];
        vertices[dst_idx + 4] = CUBE_VERTICES[src_idx + 4];
        vertices[dst_idx + 5] = CUBE_VERTICES[src_idx + 5];
    }

    indices[*n_indices + 0] = base_n_vertex + 0;
    indices[*n_indices + 1] = base_n_vertex + 1;
    indices[*n_indices + 2] = base_n_vertex + 2;
    indices[*n_indices + 3] = base_n_vertex + 3;
    indices[*n_indices + 4] = base_n_vertex + 2;
    indices[*n_indices + 5] = base_n_vertex + 1;

    *n_indices += 6;
    *n_vertices += 24;
}

static bool _is_face_visible(const struct Chunk *chunk, const struct Chunk *nearby[6], const int x,
                             const int y, const int z, const enum CubeFace face) {
    const int directions[6][3] = {
        {0, -1, 0},  // FACE_FRONT
        {1, 0, 0},   // FACE_RIGHT
        {0, 0, 1},   // FACE_TOP
        {0, 1, 0},   // FACE_BACK
        {-1, 0, 0},  // FACE_LEFT
        {0, 0, -1},  // FACE_BOTTOM
    };
    const int nx = x + directions[face][0];
    const int ny = y + directions[face][1];
    const int nz = z + directions[face][2];

    if (nx >= 0 && nx < CHUNK_SIZE && ny >= 0 && ny < CHUNK_SIZE && nz >= 0 && nz < CHUNK_SIZE)
        return !chunk->data[nz][ny][nx];

    if (nearby[face] == NULL) return true;

    const int neighbor_x = (nx + CHUNK_SIZE) % CHUNK_SIZE;
    const int neighbor_y = (ny + CHUNK_SIZE) % CHUNK_SIZE;
    const int neighbor_z = (nz + CHUNK_SIZE) % CHUNK_SIZE;

    return !nearby[face]->data[neighbor_z][neighbor_y][neighbor_x];
}

static void _add_visible_faces(const struct Chunk *chunk, const struct Chunk *nearby[6],
                               float *vertices, unsigned *n_vertices, unsigned *indices,
                               unsigned *n_indices, const int x, const int y, const int z) {
    if (!chunk->data[z][y][x]) return;

    for (enum CubeFace face = 0; face < 6; face++) {
        if (_is_face_visible(chunk, nearby, x, y, z, face)) {
            _append_face_vert_to_arr(vertices, n_vertices, indices, n_indices, x, y, z, face);
        }
    }
}

void chunk_init(struct Chunk *chunk, const int x, const int y, const int z) {
    chunk->modified = false;
    chunk->modified_unsaved = false;
    chunk->x = x;
    chunk->y = y;
    chunk->z = z;
    create_mesh(&chunk->mesh);
}

void chunk_destroy(struct Chunk *chunk) {
    delete_mesh(&chunk->mesh);
}

void chunk_gen(struct Chunk *chunk) {
    for (int ch_x = 0; ch_x < CHUNK_SIZE; ch_x++) {
        for (int ch_y = 0; ch_y < CHUNK_SIZE; ch_y++) {
            for (int ch_z = 0; ch_z < CHUNK_SIZE; ch_z++) {
                int x = chunk->x * CHUNK_SIZE + ch_x;
                int y = chunk->y * CHUNK_SIZE + ch_y;
                int z = chunk->z * CHUNK_SIZE + ch_z;
                chunk->data[ch_z][ch_y][ch_x] = (WORLD_GEN_FUNC(x, z) > y) ? 1 : 0;
            }
        }
    }

    chunk->modified_unsaved = true;
}

void chunk_mesh_update(struct Chunk *chunk, const struct Chunk *nearby[6]) {
    const size_t max_vertices = CHUNK_VOLUME * 6 * 6 * 4;
    const size_t max_indices = CHUNK_VOLUME * 6 * 6;
    chunk->modified = false;

    unsigned n_indices = 0;
    unsigned n_vertices = 0;

    float *vertices = malloc(sizeof(float) * max_vertices);
    if (vertices == NULL) return;
    unsigned *indices = malloc(sizeof(unsigned) * max_indices);
    if (indices == NULL) {
        free(vertices);
        return;
    }

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                _add_visible_faces(chunk, nearby, vertices, &n_vertices, indices, &n_indices, x, y,
                                   z);
            }
        }
    }

    upload_mesh_data(&chunk->mesh, vertices, n_vertices, indices, n_indices);

    free(vertices);
    free(indices);
}
