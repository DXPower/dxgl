#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;

out vec2 tex_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 pos = projection * view * model * vec4(pos_in, 1.0);
    
    gl_Position = pos;
    tex_pos = tex_in;
}