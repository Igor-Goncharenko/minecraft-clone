#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

#include "gfx.h"

struct Key {
    bool down, prev, toggle;
};

struct WindowState {
    int width, height;
    float mouse_xoffset, mouse_yoffset;

    struct Key keys[GLFW_KEY_LAST + 1];
};

int glfw_window_init(GLFWwindow **handle, struct WindowState *state);

#endif /* WINDOW_H */
