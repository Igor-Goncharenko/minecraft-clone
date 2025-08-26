#ifndef CHUNK_H
#define CHUNK_H

#include "gfx.h"

struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    unsigned int index_count;
};

void create_mesh(struct Mesh *mesh);

void upload_mesh_data(struct Mesh *mesh, const float *vertices, const size_t vertex_count,
                      const unsigned int *indices, const size_t index_count);

void delete_mesh(struct Mesh *mesh);

void mesh_bind(const struct Mesh *mesh);

#endif /* CHUNK_H */
