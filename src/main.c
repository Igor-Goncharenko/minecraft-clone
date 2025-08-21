#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gfx.h"
#include "renderer.h"
#include "window.h"

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
        if (state.keys[GLFW_KEY_ESCAPE].down) {
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
        glfw_window_state_update(&state);

        camera_process_input(&cam, &state);
        camera_update(&cam, state.width, state.height);

        renderer_render(&renderer, &cam, state.keys[GLFW_KEY_Y].toggle);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer_destroy(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
