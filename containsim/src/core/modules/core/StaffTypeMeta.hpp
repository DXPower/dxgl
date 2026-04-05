#pragma once

#include <modules/core/Staff.hpp>

#include <dxgl/Texture.hpp>

#include <flecs.h>

#include <generator>

namespace core {
    struct StaffTypeMeta {
        std::string type{};
        std::string display_name{};
        std::string prefab{};
        dxgl::TextureView spritesheet{};
        std::string spritesheet_path{};
        glm::ivec2 sprite_origin{};
        glm::ivec2 sprite_size{};
    };

    std::generator<StaffTypeMeta> LoadStaffTypeMetasFromFile(flecs::world& world, std::string_view file);
}