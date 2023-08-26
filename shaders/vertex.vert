#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec3 color_in;
layout (location = 2) in vec2 tex_in;

out vec3 vert_color;
out vec2 tex_pos;

uniform mat4 transform;

void main() {
    vec4 pos = transform * vec4(pos_in, 1.0);
    
    gl_Position = pos;
    vert_color = color_in;
    tex_pos = tex_in;
}