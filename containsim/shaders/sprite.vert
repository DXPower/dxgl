#version 330 core

layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec2 tex_pos;
layout (location = 2) in mat3 world_mat;
layout (location = 5) in mat3 cutout_mat;

out SpriteData {
    vec2 tex_pos;
} vs_out;

layout (std140) uniform camera {
    mat4 camera_matrix;
    mat4 projection_matrix;
};

void main() {
    vec3 world_vert_pos = world_mat * vec3(vert_pos, 1);

    gl_Position = camera_matrix * vec4(world_vert_pos, 1);

    vec2 sheet_pos = (cutout_mat * vec3(tex_pos, 1)).xy;
    vs_out.tex_pos = sheet_pos;
}