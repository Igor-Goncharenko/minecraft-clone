#include "renderer.h"

#include <stdbool.h>

#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "world.h"

#ifdef MINECRAFT_DEBUG
#define VERTEX_SHADER PROJECT_ROOT "/shaders/basic.vs"
#define FRAGMENT_SHADER PROJECT_ROOT "/shaders/basic.fs"
#else
#define VERTEX_SHADER "shaders/basic.vs"
#define FRAGMENT_SHADER "shaders/basic.fs"
#endif

static void _chunk_render(const struct Renderer *renderer, const struct Chunk *chunk) {
    shader_uniform_int(renderer->shader, "chunk_x", chunk->x);
    shader_uniform_int(renderer->shader, "chunk_y", chunk->y);
    shader_uniform_int(renderer->shader, "chunk_z", chunk->z);

    mesh_draw(&chunk->mesh);
}

static void _world_render(const struct Renderer *renderer, const struct World *world) {
    shader_bind(renderer->shader);

    for (int x = 0; x < LOADED_SIDE; x++) {
        for (int y = 0; y < LOADED_SIDE; y++) {
            for (int z = 0; z < LOADED_SIDE; z++) {
                int idx = z * LOADED_SIDE * LOADED_SIDE + y * LOADED_SIDE + x;
                _chunk_render(renderer, &world->chunks[idx]);
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
}
