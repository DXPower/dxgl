#version 330 core
in vec2 frag_tex_coords;

out vec4 frag_color;

uniform sampler2D screen_tex;

void main() { 
    frag_color = texture(screen_tex, frag_tex_coords);
}