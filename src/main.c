#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gfx.h"
#include "renderer.h"
#include "window.h"
#include "world.h"

int main(void) {
    GLFWwindow *window = NULL;
    struct WindowState state = {0};
    struct Camera cam;
    struct Renderer renderer;
    struct World world = {0};

    if (!glfw_window_init(&window, &state)) {
        fprintf(stderr, "Failed to initialize glfw window.\n");
        return EXIT_FAILURE;
    }

    camera_init(&cam, state.width, state.height);
    renderer_init(&renderer);
    if (load_world(PROJECT_ROOT "/build/world.sqlite", &world)) goto cleanup;

    while (!glfwWindowShouldClose(window)) {
        if (state.keys[GLFW_KEY_ESCAPE].down) {
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
        glfw_window_state_update(&state);

        camera_process_input(&cam, &state);
        camera_update(&cam, state.width, state.height);

        renderer_render(&renderer, &cam, &world, state.keys[GLFW_KEY_Y].toggle);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

cleanup:
    renderer_destroy(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
    close_world(&world);
    return EXIT_SUCCESS;
}
