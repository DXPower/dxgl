#version 330 core

in vec3 frag_normal;
in vec3 frag_pos;

out vec4 frag_color;

uniform vec3 view_pos;
uniform samplerCube skybox;

void main() {             
    vec3 I = normalize(frag_pos - view_pos);
    vec3 R = reflect(I, normalize(frag_normal));
    frag_color = vec4(texture(skybox, vec3(R.x, -R.y, R.z)).rgb, 1.0);
}