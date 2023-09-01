#include "Material.hpp"
#include "Uniform.hpp"

#include <functional>

void Uniform::Set(Program& program, dxtl::cstring_view name, const Material& mat) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };

    SetUniform("ambient", mat.ambient);
    SetUniform("diffuse", mat.diffuse);
    SetUniform("specular", mat.specular);
    SetUniform("shininess", mat.shininess);
}