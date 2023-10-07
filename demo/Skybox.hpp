#pragma once

#include "Cubemap.hpp"
#include "Shader.hpp"
#include "Vao.hpp"

class Camera;

namespace dxgl {
    struct Skybox {
        VaoRef vao{};
        CubemapRef cubemap{};
        mutable ProgramRef program{};

        void Render(const Camera& camera) const;
    };
}