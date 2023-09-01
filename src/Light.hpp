#pragma once

#include <glm/glm.hpp>
#include <dxtl/cstring_view.hpp>

struct Light {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class Program;

namespace Uniform {
    void Set(Program& program, dxtl::cstring_view name, const Light& light);
}