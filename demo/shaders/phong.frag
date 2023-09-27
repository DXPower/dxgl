#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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

out vec4 frag_color;

uniform Material material;
uniform Light light;
uniform vec3 view_pos;

void main() {
    // Ambient
    vec3 ambient_light = light.ambient * material.ambient;

    // Diffuse
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff_light_strength = max(dot(light_dir, norm) / 2, 0);
    vec3 diffuse_light = light.diffuse * (diff_light_strength * material.diffuse);

    // Specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec_light_strength = pow(max(dot(view_dir, reflect_dir), 0), material.shininess * 128);
    vec3 specular_light = light.specular * (spec_light_strength * material.specular);

    vec3 result = ambient_light + diffuse_light + specular_light;
    frag_color = vec4(result, 1.0);
}