#include <dxgl/Light.hpp>
#include <dxgl/Uniform.hpp>

using namespace dxgl;

void Uniform::Set(Program& program, dxtl::cstring_view name, const DirectionalLight& light) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };

    SetUniform("color.ambient", light.color.ambient);
    SetUniform("color.diffuse", light.color.diffuse);
    SetUniform("color.specular", light.color.specular);
    SetUniform("direction", light.direction);
}

void Uniform::Set(Program& program, dxtl::cstring_view name, const PointLight& light) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };

    SetUniform("color.ambient", light.color.ambient);
    SetUniform("color.diffuse", light.color.diffuse);
    SetUniform("color.specular", light.color.specular);
    SetUniform("position", light.position);
    SetUniform("constant", light.attenuation.constant);
    SetUniform("linear", light.attenuation.linear);
    SetUniform("quadratic", light.attenuation.quadratic);
}


void Uniform::Set(Program& program, dxtl::cstring_view name, const Spotlight& light) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };

    SetUniform("color.ambient", light.color.ambient);
    SetUniform("color.diffuse", light.color.diffuse);
    SetUniform("color.specular", light.color.specular);
    SetUniform("position", light.position);
    SetUniform("direction", light.direction);
    SetUniform("inner_cutoff", glm::cos(glm::radians(light.inner_cutoff)));
    SetUniform("outer_cutoff", glm::cos(glm::radians(light.outer_cutoff)));
}
