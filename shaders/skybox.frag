#version 330 core

in vec3 frag_tex_coords;

out vec4 frag_color;

uniform samplerCube skybox;

void main() {    
    frag_color = texture(skybox, vec3(frag_tex_coords.x, -frag_tex_coords.y, frag_tex_coords.z));
}