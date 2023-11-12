#version 330 core

layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec2 tex_pos;
layout (location = 2) in mat3 world_mat;
layout (location = 5) in mat3 cutout_mat;

out SpriteData {
    vec2 tex_pos;
    // vec2 test;
} vs_out;

layout (std140) uniform camera {
    mat4 camera_matrix;
    mat4 projection_matrix;
};

// mat3 BuildTranslation(vec3 delta)
// {
//     return mat3(
//         vec3(1.0, 0.0, 0.0),
//         vec3(0.0, 1.0, 0.0),
//         vec3(delta));
// }

void main() {
    // mat3 trans = BuildTranslation(vec3(700, 200, 1));

    vec3 world_vert_pos = world_mat * vec3(vert_pos, 1);
    // vs_out.test = world_vert_pos;

    // vs_out.tex_pos = world_vert_pos.xy;
    gl_Position = camera_matrix * vec4(world_vert_pos, 1);

    vec2 sheet_pos = (cutout_mat * vec3(tex_pos, 1)).xy;
    vs_out.tex_pos = sheet_pos;
}