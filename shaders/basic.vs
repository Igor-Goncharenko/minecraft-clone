#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform int chunk_x;
uniform int chunk_y;
uniform int chunk_z;

uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 model = mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

    vec3 chunk_coord = vec3(chunk_x, chunk_y, chunk_z) * 16.0f;

    vec3 final_pos = aPos + chunk_coord;
    FragPos = vec3(model * vec4(final_pos, 1.0f));
    Normal = aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0f);
}
