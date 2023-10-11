#version 330 core

in FragData {
    vec4 color;
    vec2 tex_pos;
} fs_in;

out vec4 frag_color;

uniform sampler2D spritesheet; 

void main() {
    // frag_color = fs_in.color;
    // frag_color = vec4(fs_in.tex_pos, 0, 1);
    frag_color = texture(spritesheet, fs_in.tex_pos);
}

