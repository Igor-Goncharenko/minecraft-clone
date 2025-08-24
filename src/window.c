#include "window.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"

#define UNUSED(x) (void)(x)

#define WINDOW_START_WIDTH 800
#define WINDOW_START_HEIGHT 600

static void _glfw_error_cb(int error, const char *description) {
    fprintf(stderr, "GLFW ERROR %d: %s\n", error, description);
}

static void _glfw_framebuffer_size_cb(GLFWwindow *window, int width, int height) {
    UNUSED(window);

    struct WindowState *state = glfwGetWindowUserPointer(window);

    state->width = width;
    state->height = height;

    state->half_width = width / 2.0f;
    state->half_height = height / 2.0f;

    glViewport(0, 0, width, height);
}

static void _window_state_base_init(struct WindowState *state) {
    memset(state, 0, sizeof(struct WindowState));

    state->width = WINDOW_START_WIDTH;
    state->height = WINDOW_START_HEIGHT;

    state->half_width = WINDOW_START_WIDTH / 2.0f;
    state->half_height = WINDOW_START_HEIGHT / 2.0f;

    state->mouse.xpos = state->half_width;
    state->mouse.ypos = state->half_height;
    state->mouse.last_xpos = state->half_width;
    state->mouse.last_ypos = state->half_height;

    state->mouse.grabbed = true;
}

static void _glfw_key_cb(GLFWwindow *handle, int key, int scancode, int action, int mods) {
    UNUSED(scancode);
    UNUSED(mods);

    if (key < 0) return;

    struct WindowState *state = glfwGetWindowUserPointer(handle);

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

static void _glfw_mouse_cursor_cb(GLFWwindow *handle, double xpos, double ypos) {
    struct WindowState *state = glfwGetWindowUserPointer(handle);

    if (state->mouse.grabbed) {
        glfwSetCursorPos(handle, state->half_width, state->half_height);
        state->mouse.last_xpos = state->mouse.xpos;
        state->mouse.last_ypos = state->mouse.ypos;
    }

    state->mouse.xpos = xpos;
    state->mouse.ypos = ypos;
}

int glfw_window_init(GLFWwindow **handle, struct WindowState *state) {
    glfwSetErrorCallback(_glfw_error_cb);

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
    glfwSetFramebufferSizeCallback(*handle, _glfw_framebuffer_size_cb);
    glfwSetCursorPosCallback(*handle, _glfw_mouse_cursor_cb);
    glfwSetKeyCallback(*handle, _glfw_key_cb);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD ERROR: cannot load glad.\n");
        glfwTerminate();
        return 0;
    }

    glfwSwapInterval(1);

    _window_state_base_init(state);
    glfwSetWindowUserPointer(*handle, state);

    glfwSetInputMode(*handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    return 1;
}

void glfw_window_state_update(struct WindowState *state) {
    if (state->mouse.grabbed) {
        state->mouse.xoffset = state->mouse.xpos - state->mouse.last_xpos;
        state->mouse.yoffset = state->mouse.ypos - state->mouse.last_ypos;
        state->mouse.xpos = state->mouse.last_xpos;
        state->mouse.ypos = state->mouse.last_ypos;
    }
}
