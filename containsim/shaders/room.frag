#version 330 core

in GsFsData {
    float alpha;
} fs_in;

out vec4 frag_color;

uniform vec4 color;

void main() { 
    frag_color = vec4(color.xyz, fs_in.alpha);
}
