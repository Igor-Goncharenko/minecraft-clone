#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <sqlite3.h>

#include "window.h"

#define CAM_DEFAULT_SPEED 0.05f
#define CAM_SPEED_MULTIPIER 4
#define CAM_DEFAULT_SENSIVITY 0.3f

struct Camera {
    vec3 pos;
    vec3 front, up, right;

    float pitch, yaw;

    mat4 view, proj;

    float speed, sensivity;

    int chunk_x;
    int chunk_y;
    int chunk_z;

    sqlite3 *db;
};

void camera_update(struct Camera *cam, const int scr_width, const int scr_height);
void camera_process_input(struct Camera *cam, const struct WindowState *state);
int camera_init(struct Camera *cam, sqlite3 *db, const int scr_width, const int scr_height);
int camera_save(struct Camera *cam);

#endif /* CAMERA_H */
