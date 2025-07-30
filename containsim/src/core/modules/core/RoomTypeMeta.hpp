#pragma once

#include <modules/core/Room.hpp>
#include <common/IdRegistry.hpp>
#include <glm/vec3.hpp>

#include <generator>

namespace core {
    struct RoomTypeMeta {
        std::string type{};
        std::string display_name{};
        glm::vec3 color{};
    };

    class RoomTypeMetas {
        IdRegistry m_ids{};
        std::unordered_map<RoomType, RoomTypeMeta> m_metas{};
    public:
        RoomTypeMetas() = default;
        
        RoomType Add(RoomTypeMeta meta);
        const RoomTypeMeta* Get(RoomType id) const;
        const RoomTypeMeta* Get(std::string_view type) const;
        
        std::optional<RoomType> GetIdOfType(std::string_view type) const;

        const auto& GetMetas() const { return m_metas; }
    };

    std::generator<RoomTypeMeta> LoadRoomTypeMetasFromFile(std::string_view file);
}