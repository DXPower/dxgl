#version 330 core
out vec4 frag_color;

in FRAG_DATA {
    vec3 color;
} fs_in;

void main() {
    frag_color = vec4(fs_in.color.rgb, 1);   
}  