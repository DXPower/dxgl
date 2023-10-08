#version 330 core

layout (location = 0) in vec2 block_vert_pos;
layout (location = 1) in vec2 vert_tex_pos;
layout (location = 2) in vec4 block_color;
layout (location = 3) in vec2 block_sprite_origin;

out BlockData {
    vec4 color;
    vec2 tex_pos;
} vs_out;

uniform vec2 chunk_origin;
uniform ivec2 chunk_size; 
uniform vec2 block_size;
uniform vec2 block_sprite_size;

layout (std140) uniform camera {
    mat4 camera_matrix;
};

void main() {
    vec2 block_coord;
    block_coord.x = gl_InstanceID % chunk_size.x;
    block_coord.y = floor(float(gl_InstanceID) / float(chunk_size.x));

    vec2 pos = chunk_origin + (block_coord * block_size) + (block_vert_pos * block_size);

    gl_Position = camera_matrix * vec4(pos, 0, 1);

    vs_out.color = block_color;
    vs_out.tex_pos = block_sprite_origin + (vert_tex_pos * block_sprite_size);
}