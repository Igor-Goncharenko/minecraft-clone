#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "gfx.h"
#include "shader.h"

struct VertexBuffer {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

struct Renderer {
    struct VertexBuffer cube;
    shader_t shader;
    unsigned total_points;
};

int renderer_init(struct Renderer *renderer);
void renderer_render(const struct Renderer *renderer, const struct Camera *cam, bool is_wireframe);
void renderer_destroy(struct Renderer *renderer);

#endif /* RENDERER_H */
