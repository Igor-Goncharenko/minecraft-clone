#ifndef SHADER_H
#define SHADER_H

#include <cglm/cglm.h>

#include "gfx.h"

typedef GLuint shader_t;

shader_t shader_create(const char *vs_path, const char *fs_path);
void shader_destroy(shader_t shader);
void shader_bind(const shader_t shader);
void shader_uniform_mat4(const shader_t shader_id, const char *name, const mat4 m);

#endif /* SHADER_H */
