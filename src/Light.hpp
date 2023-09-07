#pragma once

#include "LightColor.hpp"

#include <dxtl/cstring_view.hpp>

struct DirectionalLight {
    LightColor color{};
    glm::vec3 direction;
};

struct Attenuation {
    float constant{};
    float linear{};
    float quadratic{};
};

struct PointLight {
    LightColor color{};
    glm::vec3 position{};
    Attenuation attenuation{};
};

struct Spotlight {
    LightColor color{};
    glm::vec3 position{};
    glm::vec3 direction{};
    float inner_cutoff{}; // Degrees
    float outer_cutoff{}; // Degrees
};

namespace Attenuations {
    constexpr Attenuation D7   {1.0, 0.7,    1.8};
    constexpr Attenuation D13  {1.0, 0.35,   0.44};
    constexpr Attenuation D20  {1.0, 0.22,   0.20};
    constexpr Attenuation D32  {1.0, 0.14,   0.07};
    constexpr Attenuation D50  {1.0, 0.09,   0.032};
    constexpr Attenuation D65  {1.0, 0.07,   0.017};
    constexpr Attenuation D100 {1.0, 0.045,  0.0075};
    constexpr Attenuation D160 {1.0, 0.027,  0.0028};
    constexpr Attenuation D200 {1.0, 0.022,  0.0019};
    constexpr Attenuation D325 {1.0, 0.014,  0.0007};
    constexpr Attenuation D600 {1.0, 0.007,  0.0002};
    constexpr Attenuation D3250{1.0, 0.0014, 0.000007};
}

class Program;

namespace Uniform {
    void Set(Program& program, dxtl::cstring_view name, const DirectionalLight& light);
    void Set(Program& program, dxtl::cstring_view name, const PointLight& light);
    void Set(Program& program, dxtl::cstring_view name, const Spotlight& light);
}