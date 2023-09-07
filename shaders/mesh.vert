#version 330 core
layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in vec3 vert_color;
layout (location = 3) in vec2 tex_coords;

out vec3 frag_pos;
out vec3 frag_normal;
out vec3 frag_color_in;
out vec2 frag_tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(vert_pos, 1.0);
    
    frag_pos = vec3(model * vec4(vert_pos, 1.0));
    frag_normal = mat3(transpose(inverse(model))) * vert_normal;
    frag_color_in = vert_color;
    frag_tex_coords = tex_coords;
} 