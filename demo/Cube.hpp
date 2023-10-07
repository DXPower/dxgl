#pragma once

#include "Vao.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

#include <glm/glm.hpp>

namespace dxgl {
    struct Cube {
        VaoRef vao{};
        mutable ProgramRef program{};

        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale{1, 1, 1};

        void Render(const Camera& camera) const;
    };
}