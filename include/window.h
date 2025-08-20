#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

#include "gfx.h"

struct WindowState {
    int width, height;
    float mouse_xoffset, mouse_yoffset;
    bool is_wireframe, is_wireframe_prev;
};

int glfw_window_init(GLFWwindow **handle, struct WindowState *state);

#endif /* WINDOW_H */
