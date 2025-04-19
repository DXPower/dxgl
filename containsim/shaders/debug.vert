#version 330 core

layout (location = 0) in vec2 world_vert_pos;

layout (std140) uniform camera {
    mat4 camera_matrix;
    mat4 projection_matrix;
};

void main() {
    gl_Position = camera_matrix * vec4(world_vert_pos.xy, 1, 1);
}