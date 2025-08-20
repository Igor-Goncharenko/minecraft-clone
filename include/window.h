#ifndef WINDOW_H
#define WINDOW_H

#include "gfx.h"

int glfw_window_init(GLFWwindow **handle);

extern int scr_width;
extern int scr_height;
extern float scr_xoffset;
extern float scr_yoffset;

#endif /* WINDOW_H */
