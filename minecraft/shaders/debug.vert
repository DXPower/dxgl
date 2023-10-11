#version 330 core

layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec4 vert_color;

out vec4 vs_color;

layout (std140) uniform camera {
    mat4 camera_matrix;
    mat4 projection_matrix;
};

void main() {
    gl_Position = projection_matrix * vec4(vert_pos, 1, 1);
    vs_color = vert_color;
}