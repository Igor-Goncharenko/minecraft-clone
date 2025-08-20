#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gfx.h"
#include "shader.h"
#include "window.h"

#define VERTEX_SHADER PROJECT_ROOT "/shaders/basic.vs"
#define FRAGMENT_SHADER PROJECT_ROOT "/shaders/basic.fs"

static void process_input(GLFWwindow *window) {
    struct WindowState *state = glfwGetWindowUserPointer(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS &&
        state->is_wireframe == state->is_wireframe_prev)
        state->is_wireframe = !state->is_wireframe;
    else if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
        state->is_wireframe_prev = state->is_wireframe;
}

int main(void) {
    GLFWwindow *window = NULL;
    struct WindowState state = {0};
    struct Camera cam;

    if (!glfw_window_init(&window, &state)) {
        fprintf(stderr, "Failed to initialize glfw window.\n");
        return EXIT_FAILURE;
    }

    shader_t shader = shader_create(VERTEX_SHADER, FRAGMENT_SHADER);

    // clang-format off
    static const float cube_vertices[] = {
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
    
    static const unsigned cube_indices[] = {
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

    camera_init(&cam, state.width, state.height);

    unsigned VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    shader_bind(shader);
    shader_uniform_3_floats(shader, "dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader_uniform_3_floats(shader, "dirLight.ambient", 0.1f, 0.1f, 0.1f);
    shader_uniform_3_floats(shader, "dirLight.diffuse", 0.7f, 0.7f, 0.7f);
    shader_uniform_3_floats(shader, "dirLight.specular", 0.9f, 0.9f, 0.9f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    mat4 model;
    glm_mat4_identity(model);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);
        camera_process_input(&cam, window, state.mouse_xoffset, state.mouse_yoffset);
        camera_update(&cam, state.width, state.height);

        state.mouse_xoffset = 0.0f;
        state.mouse_yoffset = 0.0f;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, state.is_wireframe ? GL_LINE : GL_FILL);

        shader_bind(shader);

        shader_uniform_mat4(shader, "model", model);
        shader_uniform_mat4(shader, "view", cam.view);
        shader_uniform_mat4(shader, "projection", cam.proj);

        shader_uniform_vec3(shader, "viewPos", cam.pos);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shader_destroy(shader);

    glfwDestroyWindow(window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
