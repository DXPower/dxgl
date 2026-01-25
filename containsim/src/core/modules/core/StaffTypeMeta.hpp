#pragma once

#include <modules/core/Staff.hpp>
#include <common/IdRegistry.hpp>

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

    class StaffTypeMetas {
        IdRegistry m_ids{};
        std::unordered_map<StaffType, StaffTypeMeta> m_metas{};
    public:
        StaffTypeMetas() = default;
        
        StaffType Add(StaffTypeMeta meta);
        const StaffTypeMeta* Get(StaffType id) const;
        const StaffTypeMeta* Get(std::string_view type) const;
        
        std::optional<StaffType> GetIdOfType(std::string_view type) const;

        const auto& GetMetas() const { return m_metas; }
    };

    std::generator<StaffTypeMeta> LoadStaffTypeMetasFromFile(flecs::world& world, std::string_view file);
}