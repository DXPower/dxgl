#pragma once

#include "Vao.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

#include <glm/glm.hpp>

struct Cube {
    VaoRef vao{};
    mutable ProgramRef program{};

    glm::vec3 position{};
    // glm::vec3 rotation{};

    void Render(const Camera& camera) const;
};