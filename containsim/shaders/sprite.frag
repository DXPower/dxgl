#version 330 core

in SpriteData {
    vec2 tex_pos;
} fs_in;

out vec4 frag_color;

uniform sampler2D spritesheet; 

void main() {
    frag_color = texture(spritesheet, fs_in.tex_pos);
}

