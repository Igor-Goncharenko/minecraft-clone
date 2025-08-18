#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

struct Camera {
    vec3 pos;
    vec3 front, up, right;

    float pitch, yaw;

    mat4 view, proj;
};

void camera_update(struct Camera *cam, const int scr_width, const int scr_height);
void camera_init(struct Camera *cam, const int scr_width, const int scr_height);

#endif /* CAMERA_H */
