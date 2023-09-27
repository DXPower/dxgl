#version 330 core

out vec4 frag_color;

uniform vec3 our_color;

void main() {
    frag_color = vec4(our_color, 1);
} 