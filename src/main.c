#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gfx.h"
#include "shader.h"

#define UNUSED(x) (void)(x)

#define VERTEX_SHADER PROJECT_ROOT "/shaders/basic.vs"
#define FRAGMENT_SHADER PROJECT_ROOT "/shaders/basic.fs"

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

static void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    scr_xoffset = xpos - scr_width / 2.0f;
    scr_yoffset = ypos - scr_height / 2.0f;
    glfwSetCursorPos(window, scr_width / 2.0, scr_height / 2.0);
}

int main(void) {
    GLFWwindow *window = NULL;
    struct Camera cam;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw.");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(800, 600, "Square", NULL, NULL);

    if (!window) {
        fprintf(stderr, "Failed to create glfw window.");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD ERROR: cannot load glad.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSwapInterval(1);

    shader_t shader = shader_create(VERTEX_SHADER, FRAGMENT_SHADER);

    float vertices[] = {
        0.5f,  0.5f,  0.0f,  // top right
        0.5f,  -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f, 0.5f,  0.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };

    camera_init(&cam, scr_width, scr_height);

    unsigned VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    mat4 model;
    glm_mat4_identity(model);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);
        camera_process_input(&cam, window, scr_xoffset, scr_yoffset);
        camera_update(&cam, scr_width, scr_height);

        scr_xoffset = 0.0f;
        scr_yoffset = 0.0f;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_bind(shader);

        shader_uniform_mat4(shader, "model", model);
        shader_uniform_mat4(shader, "view", cam.view);
        shader_uniform_mat4(shader, "projection", cam.proj);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shader_destroy(shader);

    glfwDestroyWindow(window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
