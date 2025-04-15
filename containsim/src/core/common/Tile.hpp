#pragma once

#include <glm/vec2.hpp>
#include <string>
#include <map>

enum class TileType {
    Nothing,
    Grass,
    Dirt,
    Tile,
    Wall
};

struct TileData {
    TileType type{};

    constexpr bool operator==(const TileData& rhs) const = default;
};

using TileCoord = glm::ivec2;

enum class TileLayer {
    Subterranean,
    Ground,
    Walls,
    Ceiling
};

struct Tile {
    TileCoord coord{};
    TileLayer layer{};
    TileData data{};
};

struct TileMeta {
    TileType type{};
    TileLayer layer{};
    std::string friendly_name{};
    // TODO: Use asset manager for this
    std::string spritesheet_name{};
    glm::ivec2 sprite_origin{};
    glm::ivec2 sprite_size{};
};

std::map<TileType, TileMeta> LoadTileMetas();