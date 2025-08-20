#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

int scr_width = 800, scr_height = 600;
float scr_xoffset = 0.0f, scr_yoffset = 0.0f;

static void error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW ERROR %d: %s\n", error, description);
}

static void framebuffer_size_cb(GLFWwindow *window, int width, int height) {
    UNUSED(window);
    scr_width = width;
    scr_height = height;
    glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    scr_xoffset = xpos - scr_width / 2.0f;
    scr_yoffset = ypos - scr_height / 2.0f;
    glfwSetCursorPos(window, scr_width / 2.0, scr_height / 2.0);
}

int glfw_window_init(GLFWwindow **handle) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw.\n");
        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    *handle = glfwCreateWindow(800, 600, "Square", NULL, NULL);

    if (!(*handle)) {
        fprintf(stderr, "Failed to create glfw window.\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(*handle);
    glfwSetFramebufferSizeCallback(*handle, framebuffer_size_cb);
    glfwSetCursorPosCallback(*handle, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD ERROR: cannot load glad.\n");
        glfwTerminate();
        return 0;
    }

    glfwSwapInterval(1);

    return 1;
}
