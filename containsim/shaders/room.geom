#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in RoomData {
    vec2 inner_normal;
    float is_corner;
} gs_in[];

out GsFsData {
    float alpha;
} gs_out;

layout (std140) uniform camera {
    mat4 camera_matrix;
    mat4 projection_matrix;
};

// In world units
uniform float ombre_width;

void main() {    
    // Two base vertices of the line
    gl_Position = camera_matrix * gl_in[0].gl_Position;
    gs_out.alpha = 1.0f;
    EmitVertex();

    gl_Position = camera_matrix * gl_in[1].gl_Position;
    EmitVertex();
    
    vec4 inside_offset = vec4(gs_in[0].inner_normal * ombre_width, 0, 0);
    // Two Possibilities:
    // If not a corner, just extend the line as a quad towards the inside
    // Emit the first vertex of the quad
    gl_Position = camera_matrix * (gl_in[0].gl_Position + inside_offset);
    gs_out.alpha = 0.0f;
    EmitVertex();
    
    // Emit the second vertex of the quad
    gl_Position = camera_matrix * (gl_in[1].gl_Position + inside_offset);
    EmitVertex();
    EndPrimitive();
}  