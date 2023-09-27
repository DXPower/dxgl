#version 330 core
layout (location = 0) in vec2 vert_pos;
layout (location = 1) in vec3 vert_color;

out VS_OUT {
    vec3 color;
} vs_out;

void main() {
    gl_Position = vec4(vert_pos.x, vert_pos.y, 0.0, 1.0); 

    vs_out.color = vert_color;
}