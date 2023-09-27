#pragma once

#include <glm/glm.hpp>

namespace dxgl {
    struct LightColor {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };
}