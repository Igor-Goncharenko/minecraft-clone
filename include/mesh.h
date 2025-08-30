#ifndef MESH_H
#define MESH_H

#include "gfx.h"

struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    unsigned vertex_count;
};

void upload_mesh_data(struct Mesh *mesh, const float *vertices, const size_t vertex_count,
                      const unsigned int *indices, const size_t index_count);

void delete_mesh(struct Mesh *mesh);

void mesh_draw(const struct Mesh *mesh);

#endif /* MESH_H */
