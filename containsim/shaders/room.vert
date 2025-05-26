#version 330 core

layout (location = 0) in vec2 world_pos;
layout (location = 1) in vec2 inner_normal;
layout (location = 2) in float is_corner;

out RoomData {
    vec2 inner_normal;
    float is_corner;
} vs_out;

void main() {
    // vec3 world_vert_pos = vec3(world_pos, 1);
    // gl_Position = camera_matrix * vec4(world_pos, 1);
    gl_Position = vec4(world_pos, 1, 1);

    vs_out.inner_normal = inner_normal;
    vs_out.is_corner = is_corner;
}