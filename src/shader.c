#include "shader.h"

#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define INFO_LOG_SIZE 512

static shader_t _compile_shader(const char *src, GLenum type) {
    int ret;
    shader_t shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &src, NULL);
    glCompileShader(shader_id);

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        char info_log[INFO_LOG_SIZE];
        glGetShaderInfoLog(shader_id, INFO_LOG_SIZE, NULL, info_log);
        fprintf(stderr, "Failed to compile shader. Error %d:\n%s\n", ret, info_log);

        glDeleteShader(shader_id);
        return -1;
    }

    return shader_id;
}

static shader_t _load_n_compile_shader(const char *filepath, const GLenum type) {
    char *shader_src = NULL;
    shader_t shader_id = -1;

    if (load_file(filepath, &shader_src) < 0) {
        fprintf(stderr, "Failed to load shader '%s'.\n", filepath);
        return -1;
    }

    if ((shader_id = _compile_shader(shader_src, type)) == (shader_t)-1) {
        fprintf(stderr, "Failed to compile shader.\n");
        free(shader_src);
        return -1;
    }

    free(shader_src);
    return shader_id;
}

shader_t shader_create(const char *vs_path, const char *fs_path) {
    int ret;
    shader_t vs_shader = _load_n_compile_shader(vs_path, GL_VERTEX_SHADER);
    shader_t fs_shader = _load_n_compile_shader(fs_path, GL_FRAGMENT_SHADER);
    shader_t program = glCreateProgram();

    glAttachShader(program, vs_shader);
    glAttachShader(program, fs_shader);
    glLinkProgram(program);

    glDeleteShader(vs_shader);
    glDeleteShader(fs_shader);

    glGetProgramiv(program, GL_LINK_STATUS, &ret);
    if (!ret) {
        char info_log[INFO_LOG_SIZE];
        glGetProgramInfoLog(program, INFO_LOG_SIZE, NULL, info_log);
        fprintf(stderr, "Failed to link program. Error %d:\n%s\n", ret, info_log);

        glDeleteProgram(program);

        return -1;
    }

    return program;
}

void shader_destroy(shader_t shader) {
    glDeleteProgram(shader);
}

void shader_bind(const shader_t shader) {
    glUseProgram(shader);
}

void shader_uniform_mat4(const shader_t shader_id, const char *name, const mat4 m) {
    glUniformMatrix4fv(glGetUniformLocation(shader_id, name), 1, GL_FALSE, (const float *)m);
}

void shader_uniform_vec3(GLuint shader_id, const char *name, vec3 v3) {
    glUniform3fv(glGetUniformLocation(shader_id, name), 1, v3);
}

void shader_uniform_3_floats(GLuint shader_id, const char *name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(shader_id, name), x, y, z);
}
