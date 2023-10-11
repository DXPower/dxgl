#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in SpriteData {
    vec2 tex_pos;
    vec4 color;
} gs_in[];

out FragData {
    vec4 color;
    vec2 tex_pos;
} gs_out;

uniform vec2 half_world_size;
uniform vec2 sprite_size;

layout (std140) uniform camera {
    mat4 camera_matrix;
    mat4 projection_matrix;
};

void main() {
    gs_out.color = gs_in[0].color;

    vec4 center = gl_in[0].gl_Position;
    vec2 tex_top_left = gs_in[0].tex_pos;

    // Top left
    gl_Position = camera_matrix * (center - half_world_size);
    gs_out.tex_pos = tex_top_left;
    EmitVertex();

    // Top right
    gl_Position = camera_matrix * (center + half_world_size * vec4(vec2(1, -1), 0, 0));
    gs_out.tex_pos = tex_top_left + sprite_size * vec2(1, 0);
    EmitVertex();

    // Bottom left
    gl_Position = camera_matrix * (center + half_world_size * vec4(vec2(-1, 1), 0, 0));
    gs_out.tex_pos = tex_top_left + sprite_size * vec2(0, -1);
    EmitVertex();

    // Bottom right
    gl_Position = camera_matrix * (center + half_world_size * vec4(vec2(1, 1), 0, 0));
    gs_out.tex_pos = tex_top_left + sprite_size * vec2(1, -1);
    EmitVertex();
    EndPrimitive();
}