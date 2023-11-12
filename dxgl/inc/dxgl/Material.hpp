#pragma once

#include <glm/glm.hpp>
#include <dxtl/cstring_view.hpp>
#include "Texture.hpp"

namespace dxgl {
    struct Material {
        glm::vec3 ambient{};
        glm::vec3 diffuse{};
        glm::vec3 specular{};
        float shininess{};
    };

    struct TexMaterial {
        TextureView diffuse_map{};
        TextureView specular_map{};
        TextureView emission_map{};
        float shininess{};
    };

    class Program;

    namespace Uniform {
        void Set(Program& program, dxtl::cstring_view name, const Material& mat);
        void Set(Program& program, dxtl::cstring_view name, const TexMaterial& mat);
    }
}