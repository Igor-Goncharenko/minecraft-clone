#include "window.h"

#include <stdio.h>
#include <string.h>

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
    memset(state, 0, sizeof(struct WindowState));

    state->width = WINDOW_START_WIDTH;
    state->height = WINDOW_START_HEIGHT;
    state->mouse_xoffset = 0.0f;
    state->mouse_yoffset = 0.0f;
}

static void _key_callback(GLFWwindow *handle, int key, int scancode, int action, int mods) {
    UNUSED(scancode);
    UNUSED(mods);

    struct WindowState *state = glfwGetWindowUserPointer(handle);

    if (key < 0) return;

    switch (action) {
        case GLFW_PRESS:
            state->keys[key].down = true;
            break;
        case GLFW_RELEASE:
            state->keys[key].down = false;
            break;
        case GLFW_REPEAT:
            break;
        default:
            fprintf(stderr, "%s: Unprocessed keyboard key action: %d.\n", __FUNCTION__, action);
            break;
    }

    if (state->keys[key].down && !state->keys[key].prev)
        state->keys[key].toggle = !state->keys[key].toggle;
    state->keys[key].prev = state->keys[key].down;
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
    glfwSetKeyCallback(*handle, _key_callback);

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
