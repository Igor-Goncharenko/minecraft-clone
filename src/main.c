#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gfx.h"
#include "renderer.h"
#include "window.h"
#include "world.h"

#define DB_FILEPATH PROJECT_ROOT "/build/world.sqlite"

void db_close(sqlite3 **db) {
    if (*db != NULL) {
        sqlite3_close(*db);
        *db = NULL;
    }
    printf("DB closed\n");
}

int db_init(const char *filepath, sqlite3 **db) {
    int rc;
    char *err_msg;

    if ((rc = sqlite3_open(filepath, db)) != SQLITE_OK) {
        fprintf(stderr, "Failed to open database '%s'(%d).\n", filepath, rc);
        *db = NULL;
        return 1;
    }

    if ((rc = sqlite3_exec(*db, "PRAGMA journal_mode=WAL;", NULL, NULL, &err_msg)) != SQLITE_OK) {
        fprintf(stderr, "SQL error(%d): %s\n", rc, err_msg);
        sqlite3_free(err_msg);
        db_close(db);
        return 1;
    }

    printf("DB opened from '%s'.\n", filepath);

    return 0;
}

int main(void) {
    GLFWwindow *window = NULL;
    sqlite3 *db = NULL;
    struct WindowState state = {0};
    struct Camera cam;
    struct Renderer renderer;
    struct World world = {0};

    if (!glfw_window_init(&window, &state)) {
        fprintf(stderr, "Failed to initialize glfw window.\n");
        return EXIT_FAILURE;
    }

    // we must initialize camera first to get cam position to load chunks
    if (db_init(DB_FILEPATH, &db) || camera_init(&cam, db, state.width, state.height) ||
        load_world(db, &world, &cam))
        goto cleanup;
    renderer_init(&renderer);

    while (!glfwWindowShouldClose(window)) {
        if (state.keys[GLFW_KEY_ESCAPE].down) {
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
        glfw_window_state_update(&state);

        camera_process_input(&cam, &state);
        camera_update(&cam, state.width, state.height);

        update_world(&world, &cam);

        renderer_render(&renderer, &cam, &world, state.keys[GLFW_KEY_Y].toggle);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

cleanup:
    renderer_destroy(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
    close_world(&world);
    camera_save(&cam);
    db_close(&db);
    return EXIT_SUCCESS;
}
