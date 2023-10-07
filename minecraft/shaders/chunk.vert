#version 330 core

layout (location = 0) in vec2 block_vert_pos;
layout (location = 1) in vec4 block_color;

out BlockData {
    vec4 color;
} vs_out;

uniform vec2 chunk_origin;
uniform ivec2 chunk_size; 
uniform vec2 block_size;

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
}