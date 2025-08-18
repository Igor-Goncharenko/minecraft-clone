#include "camera.h"

#include <cglm/cglm.h>

#include "gfx.h"

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

    cam->front[0] = 0.0f;
    cam->front[1] = 0.0f;
    cam->front[2] = -1.0f;

    cam->up[0] = 0.0f;
    cam->up[1] = 1.0f;
    cam->up[2] = 0.0f;

    glm_cross(cam->front, cam->up, cam->right);

    cam->speed = CAM_DEFAULT_SPEED;

    camera_update(cam, scr_width, scr_height);
}

void camera_process_input(struct Camera *cam, GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_muladds(cam->front, cam->speed, cam->pos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_muladds(cam->front, -cam->speed, cam->pos);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_muladds(cam->right, -cam->speed, cam->pos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_muladds(cam->right, cam->speed, cam->pos);
    }
}
