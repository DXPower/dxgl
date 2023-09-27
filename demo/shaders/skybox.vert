#version 330 core

layout (location = 0) in vec3 vert_pos;

out vec3 frag_tex_coords;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 pos = projection * view * vec4(vert_pos, 1.0);
    gl_Position = pos.xyww;
    
    frag_tex_coords = vert_pos;
} 