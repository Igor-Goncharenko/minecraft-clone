#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "world.h"

struct Renderer {
    shader_t shader;
    unsigned total_points;
    struct Mesh block_mesh;
};

int renderer_init(struct Renderer *renderer);
void renderer_render(const struct Renderer *renderer, const struct Camera *cam,
                     const struct World *world, bool is_wireframe);
void renderer_destroy(struct Renderer *renderer);

#endif /* RENDERER_H */
