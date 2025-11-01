#pragma once

#include <modules/core/Tile.hpp>
#include <common/IdRegistry.hpp>

#include <dxgl/Texture.hpp>
#include <generator>

namespace core {

struct TileTypeMeta {
    std::string type{};
    std::string display_name{};
    std::string prefab{};
    TileLayer layer{};
    dxgl::TextureView spritesheet{};
    std::string spritesheet_path{};
    glm::ivec2 sprite_origin{};
    glm::ivec2 sprite_size{};
};

class TileTypeMetas {
    IdRegistry m_ids{};
    std::unordered_map<TileType, TileTypeMeta> m_metas{};
public:
    TileTypeMetas();
    
    TileType Add(TileTypeMeta meta);
    const TileTypeMeta* Get(TileType id) const;
    const TileTypeMeta* Get(std::string_view type) const;
    
    std::optional<TileType> GetIdOfType(std::string_view type) const;

    const auto& GetMetas() const { return m_metas; }
};

std::generator<TileTypeMeta> LoadTileTypeMetasFromFile(flecs::world& world, std::string_view file);

}