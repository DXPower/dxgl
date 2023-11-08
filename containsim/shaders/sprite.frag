#version 330 core

in SpriteData {
    vec2 tex_pos;
    // vec2 test;
} fs_in;

out vec4 frag_color;

uniform sampler2D spritesheet; 

void main() {
    frag_color = texture(spritesheet, fs_in.tex_pos);
    // frag_color = vec4(fs_in.tex_pos * 10, 0, 1) + texture(spritesheet, fs_in.tex_pos) * 0.001;
}

