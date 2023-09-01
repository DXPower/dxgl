#version 330 core
layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;

out vec3 frag_normal;
out vec3 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(vert_pos, 1.0);
    
    // frag_normal = mat3(transpose(inverse(model))) * vert_normal;
    frag_pos = vec3(model * vec4(vert_pos, 1.0));
    frag_normal = mat3(transpose(inverse(model))) * vert_normal;
} 