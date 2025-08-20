#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gfx.h"
#include "renderer.h"
#include "window.h"

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
    struct Renderer renderer;

    if (!glfw_window_init(&window, &state)) {
        fprintf(stderr, "Failed to initialize glfw window.\n");
        return EXIT_FAILURE;
    }

    camera_init(&cam, state.width, state.height);
    renderer_init(&renderer);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);
        camera_process_input(&cam, window, state.mouse_xoffset, state.mouse_yoffset);
        camera_update(&cam, state.width, state.height);

        state.mouse_xoffset = 0.0f;
        state.mouse_yoffset = 0.0f;

        renderer_render(&renderer, &cam, state.is_wireframe);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer_destroy(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
