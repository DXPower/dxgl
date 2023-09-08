#version 330 core

struct TexColor {
    sampler2D diffuse_map;
    sampler2D specular_map;
    // sampler2D emission_map;
    float shininess;
};

struct LightColor {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct BasicMaterial {
    LightColor color;
    TexColor tex_color;
    float shininess;
};

struct DirectionalLight {
    LightColor color;
    vec3 direction;
};

struct PointLight {
    LightColor color;
    vec3 position;
    float constant;
    float linear;
    float quadratic;
};

struct Spotlight {
    LightColor color;
    vec3 position;
    vec3 direction;
    float inner_cutoff;
    float outer_cutoff;
};

in vec3 frag_pos;
in vec3 frag_normal;
in vec3 frag_color_in;
in vec2 frag_tex_coords;

out vec4 frag_color;

uniform BasicMaterial material;
uniform vec3 view_pos;


#define NUM_POINT_LIGHTS 4

uniform DirectionalLight dir_light;
uniform Spotlight spotlight;
uniform PointLight point_lights[NUM_POINT_LIGHTS];

struct LightResult {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    // vec3 emission;
};

LightResult CalcPhongLight(LightColor color, vec3 normal, vec3 light_dir, vec3 view_dir);
LightResult CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir);
LightResult CalcPointLight(PointLight light, vec3 normal, vec3 view_dir);
LightResult CalcSpotlight(Spotlight light, vec3 normal, vec3 view_dir);

vec3 LightResultToVec(LightResult light) {
    return vec3(light.ambient + light.diffuse + light.specular);
}

void main() {
    vec3 normal = normalize(frag_normal);
    vec3 view_dir = normalize(view_pos - frag_pos);
    
    vec3 result;

    result += LightResultToVec(CalcDirectionalLight(dir_light, normal, view_dir));
    result += LightResultToVec(CalcSpotlight(spotlight, normal, view_dir));

    for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
        LightResult r = CalcPointLight(point_lights[i], normal, view_dir);
        result += LightResultToVec(r);
    }

    frag_color = vec4(result, 1);
    // frag_color = vec4(1);
}


LightResult CalcPhongLight(LightColor color, vec3 normal, vec3 light_dir, vec3 view_dir) {
    LightResult result;


    vec3 tex_color = vec3(texture(material.tex_color.diffuse_map, frag_tex_coords));
    // vec3 tex_color = vec3(1);
    vec3 mat_color_amb = material.color.ambient;

    // Ambient
    result.ambient = color.ambient * tex_color * mat_color_amb;

    // Diffuse
    vec3 mat_color_dif = material.color.diffuse;

    float diff_light_strength = max(dot(light_dir, normal), 0);
    result.diffuse = color.diffuse * (diff_light_strength * tex_color * mat_color_dif);

    // Specular
    vec3 reflect_dir = reflect(-light_dir, normal);
    vec3 tex_spec = vec3(texture(material.tex_color.specular_map, frag_tex_coords));
    // vec3 tex_spec = vec3(1);
    vec3 mat_color_spec = material.color.specular;

    float spec_light_strength = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
    result.specular = color.specular * (spec_light_strength * tex_spec * mat_color_spec);

    // // Emission
    // // result.emission = vec3(texture(material.emission_map, frag_tex_coords));

    return result;
}

LightResult CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);
    return CalcPhongLight(light.color, normal, light_dir, view_dir);
}

LightResult CalcPointLight(PointLight light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);
    LightResult result = CalcPhongLight(light.color, normal, light_dir, view_dir);

    // Attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    result.ambient *= attenuation;
    result.diffuse *= attenuation;
    result.specular *= attenuation;

    return result;
}

LightResult CalcSpotlight(Spotlight light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);
    LightResult result = CalcPhongLight(light.color, normal, light_dir, view_dir);

    // Spotlight
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.inner_cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0, 1);

    result.diffuse *= intensity;
    result.specular *= intensity;

    return result;
}