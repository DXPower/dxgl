#version 330 core

out vec4 frag_color;

in vec2 tex_pos;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform float mix_value;

void main() {
    vec4 tex_color = mix(texture(tex1, tex_pos), texture(tex2, tex_pos), mix_value);
    tex_color.a = 1;
    
    frag_color = tex_color;
} 