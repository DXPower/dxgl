#pragma once

#include "VAO.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

#include <glm/glm.hpp>

struct Cube {
    VAO vao{};
    mutable Program program{};

    glm::vec3 position{};
    // glm::vec3 rotation{};

    void Render(const Camera& camera) const;
};