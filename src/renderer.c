#include "renderer.h"

#include <stdbool.h>

#include "camera.h"
#include "shader.h"
#include "world.h"

#define VERTEX_SHADER PROJECT_ROOT "/shaders/basic.vs"
#define FRAGMENT_SHADER PROJECT_ROOT "/shaders/basic.fs"

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

static const unsigned CUBE_INDICES[] = {
     0,  1,  2,     /* front */
     3,  2,  1,
     4,  5,  6,     /* right */
     7,  6,  5,
     8,  9, 10,     /* top */
    11, 10,  9,
    12, 13, 14,     /* back */
    15, 14, 13,
    16, 17, 18,     /* left */
    19, 18, 17,
    20, 21, 22,     /* bottom */
    23, 22, 21
};
// clang-format on

static void _vertex_buffer_init(struct VertexBuffer *buff, const float *vertices,
                                const unsigned sizeof_vertices, const unsigned *indices,
                                const unsigned sizeof_indices) {
    glGenVertexArrays(1, &buff->vao);
    glGenBuffers(1, &buff->vbo);
    glGenBuffers(1, &buff->ebo);

    glBindVertexArray(buff->vao);

    glBindBuffer(GL_ARRAY_BUFFER, buff->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sizeof_vertices, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * sizeof_indices, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

static void _chunk_render(const struct Renderer *renderer, const struct Chunk *chunk) {
    glBindVertexArray(renderer->cube.vao);

    shader_uniform_int(renderer->shader, "chunk_x", chunk->x);
    shader_uniform_int(renderer->shader, "chunk_y", chunk->y);
    shader_uniform_int(renderer->shader, "chunk_z", chunk->z);

    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
        if (chunk->data[i]) {
            shader_uniform_int(renderer->shader, "block_index", i);
            glDrawElements(GL_TRIANGLES, renderer->total_points, GL_UNSIGNED_INT, 0);
        }
    }
    glBindVertexArray(0);
}

static void _world_render(const struct Renderer *renderer, const struct World *world) {
    shader_bind(renderer->shader);

    for (int x = 0; x < LOADED_SIDE; x++) {
        for (int y = 0; y < LOADED_SIDE; y++) {
            for (int z = 0; z < LOADED_SIDE; z++) {
                int idx = z * LOADED_SIDE * LOADED_SIDE + y * LOADED_SIDE + x;
                _chunk_render(renderer, &world->loaded_chunks[idx]);
            }
        }
    }
}

int renderer_init(struct Renderer *renderer) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    renderer->shader = shader_create(VERTEX_SHADER, FRAGMENT_SHADER);

    shader_bind(renderer->shader);
    shader_uniform_3_floats(renderer->shader, "dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader_uniform_3_floats(renderer->shader, "dirLight.ambient", 0.1f, 0.1f, 0.1f);
    shader_uniform_3_floats(renderer->shader, "dirLight.diffuse", 0.7f, 0.7f, 0.7f);
    shader_uniform_3_floats(renderer->shader, "dirLight.specular", 0.9f, 0.9f, 0.9f);

    _vertex_buffer_init(&renderer->cube, CUBE_VERTICES, sizeof(CUBE_VERTICES), CUBE_INDICES,
                        sizeof(CUBE_INDICES));

    renderer->total_points = sizeof(CUBE_INDICES) / sizeof(unsigned);

    return 0;
}

void renderer_render(const struct Renderer *renderer, const struct Camera *cam,
                     const struct World *world, bool is_wireframe) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, is_wireframe ? GL_LINE : GL_FILL);

    shader_bind(renderer->shader);
    shader_uniform_mat4(renderer->shader, "view", cam->view);
    shader_uniform_mat4(renderer->shader, "projection", cam->proj);
    shader_uniform_vec3(renderer->shader, "viewPos", cam->pos);

    _world_render(renderer, world);
}

void renderer_destroy(struct Renderer *renderer) {
    shader_destroy(renderer->shader);

    glDeleteVertexArrays(1, &renderer->cube.vao);
    glDeleteBuffers(1, &renderer->cube.vbo);
    glDeleteBuffers(1, &renderer->cube.ebo);
}
