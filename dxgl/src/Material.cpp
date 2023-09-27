#include <dxgl/Material.hpp>
#include <dxgl/Uniform.hpp>

#include <functional>

using namespace dxgl;

void Uniform::Set(Program& program, dxtl::cstring_view name, const Material& mat) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };

    SetUniform("ambient", mat.ambient);
    SetUniform("diffuse", mat.diffuse);
    SetUniform("specular", mat.specular);
    SetUniform("shininess", mat.shininess);
}


void Uniform::Set(Program& program, dxtl::cstring_view name, const TexMaterial& mat) {
    auto SetUniform = [&](const std::string& item_name, const auto& u) {
        Uniform::Set(program, std::string(name) + '.' + item_name, u);
    };


    SetUniform("diffuse_map", 0);
    mat.diffuse_map->Use(0);

    SetUniform("specular_map", 1);
    mat.specular_map->Use(1);

    // SetUniform("emission_map", 2);
    // mat.emission_map->Use(2);

    SetUniform("shininess", mat.shininess);
}