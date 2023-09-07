#pragma once

#include <glm/glm.hpp>

struct LightColor {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};