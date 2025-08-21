#include "camera.h"

#include <cglm/cglm.h>

#include "window.h"

static void _update_camera_direction(struct Camera *cam) {
    if (cam->pitch > 89.0f)
        cam->pitch = 89.0f;
    else if (cam->pitch < -89.0f)
        cam->pitch = -89.0f;

    cam->front[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    cam->front[1] = sin(glm_rad(cam->pitch));
    cam->front[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    glm_vec3_normalize(cam->front);

    glm_vec3_cross((vec3){0.0f, 1.0f, 0.0f}, cam->front, cam->right);
    glm_vec3_cross(cam->front, cam->right, cam->up);
}

void camera_update(struct Camera *cam, const int scr_width, const int scr_height) {
    vec3 center;
    glm_vec3_add(cam->pos, cam->front, center);
    glm_lookat(cam->pos, center, cam->up, cam->view);

    float aspect = (float)scr_width / (float)scr_height;
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 100.0f, cam->proj);
}

void camera_init(struct Camera *cam, const int scr_width, const int scr_height) {
    cam->pos[0] = 0.0f;
    cam->pos[1] = 0.0f;
    cam->pos[2] = 5.0f;

    cam->pitch = 0.0f;
    cam->yaw = -90.0f;

    _update_camera_direction(cam);

    glm_cross(cam->front, cam->up, cam->right);

    cam->speed = CAM_DEFAULT_SPEED;
    cam->sensivity = CAM_DEFAULT_SENSIVITY;

    camera_update(cam, scr_width, scr_height);
}

void camera_process_input(struct Camera *cam, const struct WindowState *state) {
    // keyboard input
    if (state->keys[GLFW_KEY_W].down) {
        glm_vec3_muladds(cam->front, cam->speed, cam->pos);
    }
    if (state->keys[GLFW_KEY_S].down) {
        glm_vec3_muladds(cam->front, -cam->speed, cam->pos);
    }
    if (state->keys[GLFW_KEY_A].down) {
        glm_vec3_muladds(cam->right, cam->speed, cam->pos);
    }
    if (state->keys[GLFW_KEY_D].down) {
        glm_vec3_muladds(cam->right, -cam->speed, cam->pos);
    }

    // mouse input
    cam->yaw += state->mouse.xoffset * cam->sensivity;
    cam->pitch -= state->mouse.yoffset * cam->sensivity;

    _update_camera_direction(cam);
}
