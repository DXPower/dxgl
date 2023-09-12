#version 330 core

in vec2 frag_tex_coords;

out vec4 FragColor;

uniform sampler2D tex;

void main() {
    vec4 tex_color = texture(tex, frag_tex_coords);
    FragColor = tex_color;
} 