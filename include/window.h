#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

#include "gfx.h"

struct Key {
    bool down, prev, toggle;
};

struct Mouse {
    bool grabbed;

    float xpos, ypos;
    float xoffset, yoffset;
    float last_xpos, last_ypos;
};

struct WindowState {
    int width, height;
    float half_width, half_height;

    struct Key keys[GLFW_KEY_LAST + 1];
    struct Mouse mouse;
};

int glfw_window_init(GLFWwindow **handle, struct WindowState *state);

void glfw_window_state_update(struct WindowState *state);

#endif /* WINDOW_H */
