#include "Light.hpp"
#include "Uniform.hpp"

void Uniform::Set(Program& program, dxtl::cstring_view name, const Light& light) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };

    SetUniform("position", light.position);
    SetUniform("ambient", light.ambient);
    SetUniform("diffuse", light.diffuse);
    SetUniform("specular", light.specular);
}
