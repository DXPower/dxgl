#version 330 core

layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec2 sprite_origin;
layout (location = 2) in vec4 color;

out SpriteData {
    vec2 tex_pos;
    vec4 color;
} vs_out;

void main() {
    gl_Position = vec4(vert_pos, 1, 1);
    vs_out.tex_pos = sprite_origin;
    vs_out.color = color;
}