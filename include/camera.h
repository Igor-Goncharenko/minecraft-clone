#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

#include "gfx.h"

#define CAM_DEFAULT_SPEED 0.05f
#define CAM_DEFAULT_SENSIVITY 0.5f

struct Camera {
    vec3 pos;
    vec3 front, up, right;

    float pitch, yaw;

    mat4 view, proj;

    float speed, sensivity;
};

void camera_update(struct Camera *cam, const int scr_width, const int scr_height);
void camera_init(struct Camera *cam, const int scr_width, const int scr_height);
void camera_process_input(struct Camera *cam, GLFWwindow *window, const float xoffset,
                          const float yoffset);

#endif /* CAMERA_H */
