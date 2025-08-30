#include "mesh.h"

#include "gfx.h"

void delete_mesh(struct Mesh *mesh) {
    if (mesh->vao != 0) {
        glDeleteVertexArrays(1, &mesh->vao);
        mesh->vao = 0;
    }
    if (mesh->vbo != 0) {
        glDeleteBuffers(1, &mesh->vbo);
        mesh->vbo = 0;
    }
    if (mesh->ebo != 0) {
        glDeleteBuffers(1, &mesh->ebo);
        mesh->ebo = 0;
    }
}

void upload_mesh_data(struct Mesh *mesh, const float *vertices, const size_t vertex_count,
                      const unsigned *indices, const size_t index_count) {
    delete_mesh(mesh);

    mesh->vertex_count = index_count;

    if (index_count == 0 || vertex_count == 0) return;

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_count, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * index_count, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void mesh_draw(const struct Mesh *mesh) {
    if (mesh->vertex_count <= 0) return;
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->vertex_count, GL_UNSIGNED_INT, 0);
}
