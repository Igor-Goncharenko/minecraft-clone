#include "camera.h"

#include <cglm/cglm.h>

#include "window.h"

static void _camera_reset_position(struct Camera *cam) {
    cam->pos[0] = 0.0f;
    cam->pos[1] = 50.0f;
    cam->pos[2] = 0.0f;

    cam->pitch = -20.0f;
    cam->yaw = 45.0f;

    printf("Camera position reseted\n");
}

static int _camera_load_data(struct Camera *cam) {
    const char *sql = "SELECT x, y, z, pitch, yaw FROM camera WHERE id = 1;";
    sqlite3_stmt *stmt;
    int rc;

    if ((rc = sqlite3_prepare_v2(cam->db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(cam->db));
        return 1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        cam->pos[0] = sqlite3_column_double(stmt, 0);
        cam->pos[1] = sqlite3_column_double(stmt, 1);
        cam->pos[2] = sqlite3_column_double(stmt, 2);
        cam->pitch = sqlite3_column_double(stmt, 3);
        cam->yaw = sqlite3_column_double(stmt, 4);
        sqlite3_finalize(stmt);
        return 0;
    } else if (rc == SQLITE_DONE) {
        _camera_reset_position(cam);
        sqlite3_finalize(stmt);
        return 0;
    } else {
        fprintf(stderr, "DB error in step: %s.\n", sqlite3_errmsg(cam->db));
        sqlite3_finalize(stmt);
        return 1;
    }
}

static int _camera_save_data(struct Camera *cam) {
    const char *sql =
        "INSERT OR REPLACE INTO camera (id, x, y, z, pitch, yaw) "
        "VALUES (1, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc;

    if ((rc = sqlite3_prepare_v2(cam->db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(cam->db));
        return 1;
    }

    sqlite3_bind_double(stmt, 1, cam->pos[0]);
    sqlite3_bind_double(stmt, 2, cam->pos[1]);
    sqlite3_bind_double(stmt, 3, cam->pos[2]);
    sqlite3_bind_double(stmt, 4, cam->pitch);
    sqlite3_bind_double(stmt, 5, cam->yaw);

    if ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        fprintf(stderr, "[cam]Execution failed: %s\n", sqlite3_errmsg(cam->db));
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

static int _camera_init_table(sqlite3 *db) {
    int rc;
    char *err_msg = NULL;
    const char *sql =
        "CREATE TABLE IF NOT EXISTS camera ("
        "id INTEGER PRIMARY KEY CHECK (id = 1),"
        "x REAL NOT NULL,"
        "y REAL NOT NULL,"
        "z REAL NOT NULL,"
        "pitch REAL NOT NULL,"
        "yaw REAL NOT NULL);";

    if ((rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg)) != SQLITE_OK) {
        fprintf(stderr, "SQL error(%d): %s\n", rc, err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    printf("camera table checked/created successfully.\n");
    return 0;
}

static void _update_camera_chunk_coordinates(struct Camera *cam) {
    cam->chunk_x = (int)cam->pos[0] / 16 + ((cam->pos[0] < 0) ? -1 : 0);
    cam->chunk_y = (int)cam->pos[1] / 16 + ((cam->pos[1] < 0) ? -1 : 0);
    cam->chunk_z = (int)cam->pos[2] / 16 + ((cam->pos[2] < 0) ? -1 : 0);
}

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
    glm_vec3_normalize(cam->right);
    glm_vec3_cross(cam->front, cam->right, cam->up);
}

void camera_update(struct Camera *cam, const int scr_width, const int scr_height) {
    vec3 center;
    glm_vec3_add(cam->pos, cam->front, center);
    glm_lookat(cam->pos, center, cam->up, cam->view);

    float aspect = (float)scr_width / (float)scr_height;
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 1000.0f, cam->proj);

    _update_camera_direction(cam);
    _update_camera_chunk_coordinates(cam);
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

    if (state->keys[GLFW_KEY_LEFT_SHIFT].down) {
        cam->speed = CAM_DEFAULT_SPEED * CAM_SPEED_MULTIPIER;
    } else {
        cam->speed = CAM_DEFAULT_SPEED;
    }

    if (state->keys[GLFW_KEY_R].down) {
        _camera_reset_position(cam);
    }

    // mouse input
    cam->yaw += state->mouse.xoffset * cam->sensivity;
    cam->pitch -= state->mouse.yoffset * cam->sensivity;
}

int camera_init(struct Camera *cam, sqlite3 *db, const int scr_width, const int scr_height) {
    cam->db = db;
    if (_camera_init_table(db) || _camera_load_data(cam)) return 1;

    cam->speed = CAM_DEFAULT_SPEED;
    cam->sensivity = CAM_DEFAULT_SENSIVITY;

    camera_update(cam, scr_width, scr_height);

    return 0;
}

int camera_save(struct Camera *cam) {
    return _camera_save_data(cam);
}
