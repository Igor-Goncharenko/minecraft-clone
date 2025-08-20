#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

#define WINDOW_START_WIDTH 800
#define WINDOW_START_HEIGHT 600

static void error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW ERROR %d: %s\n", error, description);
}

static void framebuffer_size_cb(GLFWwindow *window, int width, int height) {
    UNUSED(window);

    struct WindowState *state = glfwGetWindowUserPointer(window);

    state->width = width;
    state->height = height;

    glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    struct WindowState *state = glfwGetWindowUserPointer(window);

    state->mouse_xoffset = xpos - state->width / 2.0f;
    state->mouse_yoffset = ypos - state->height / 2.0f;

    glfwSetCursorPos(window, state->width / 2.0f, state->height / 2.0f);
}

static void window_state_base_init(struct WindowState *state) {
    state->width = WINDOW_START_WIDTH;
    state->height = WINDOW_START_HEIGHT;
    state->mouse_xoffset = 0.0f;
    state->mouse_yoffset = 0.0f;
    state->is_wireframe = false;
    state->is_wireframe_prev = true;
}

int glfw_window_init(GLFWwindow **handle, struct WindowState *state) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw.\n");
        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    *handle = glfwCreateWindow(WINDOW_START_WIDTH, WINDOW_START_HEIGHT, "Square", NULL, NULL);

    if (!(*handle)) {
        fprintf(stderr, "Failed to create glfw window.\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(*handle);
    glfwSetFramebufferSizeCallback(*handle, framebuffer_size_cb);
    glfwSetCursorPosCallback(*handle, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD ERROR: cannot load glad.\n");
        glfwTerminate();
        return 0;
    }

    glfwSwapInterval(1);

    window_state_base_init(state);
    glfwSetWindowUserPointer(*handle, state);

    return 1;
}
