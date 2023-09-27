#version 330 core

struct TexMaterial {
    sampler2D diffuse_map;
    sampler2D specular_map;
    sampler2D emission_map;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_tex_coords;

out vec4 frag_color;

uniform TexMaterial material;
uniform Light light;
uniform vec3 view_pos;

void main() {
    vec3 tex_color = vec3(texture(material.diffuse_map, frag_tex_coords));
    
    // Ambient
    vec3 ambient_light = light.ambient * tex_color;

    // Diffuse
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff_light_strength = max(dot(light_dir, norm) / 2, 0);
    vec3 diffuse_light = light.diffuse * (diff_light_strength * tex_color);

    // Specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    vec3 tex_spec = vec3(texture(material.specular_map, frag_tex_coords));
    float spec_light_strength = pow(max(dot(view_dir, reflect_dir), 0), material.shininess * 128);
    vec3 specular_light = light.specular * (spec_light_strength * tex_spec);

    // Emission
    vec3 emission_light = vec3(texture(material.emission_map, frag_tex_coords));

    if (tex_spec != vec3(0)) {
        emission_light = vec3(0);
    }

    vec3 result = ambient_light + diffuse_light + specular_light + emission_light;

    frag_color = vec4(result, 1.0);
}