#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 frag_color;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

void main() {
    float ambient_strength = 0.1;
    vec3 ambient_light = ambient_strength * light_color;

    // vec3 result = ambient_light * object_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);

    float diff_light_strength = max(dot(light_dir, norm) / 2, 0);
    vec3 diffuse_light = diff_light_strength * light_color;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float specular_strength = 0.5;
    float spec_light_strength = pow(max(dot(view_dir, reflect_dir), 0), 64);
    vec3 specular_light = specular_strength * spec_light_strength * light_color;

    vec3 result = (diffuse_light + ambient_light + specular_light) * object_color;

    frag_color = vec4(result, 1.0);
}