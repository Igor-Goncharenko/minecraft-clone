#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform int block_index;
uniform mat4 view;
uniform mat4 projection;

void main() {
    int block_x = block_index & 0xF;
    int block_y = (block_index >> 4) & 0xF;
    int block_z = block_index >> 8;

    mat4 model = mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

    vec3 final_pos = aPos + vec3(block_x, block_z, block_y);
    FragPos = vec3(model * vec4(final_pos, 1.0f));
    Normal = aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0f);
}
