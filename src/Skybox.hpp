#pragma once

#include "Cubemap.hpp"
#include "Shader.hpp"
#include "Vao.hpp"

class Camera;

struct Skybox {
    VaoRef vao{};
    CubemapRef cubemap{};
    mutable ProgramRef program{};

    void Render(const Camera& camera) const;
};