#pragma once

#include <glm/vec2.hpp>
#include <flecs.h>
#include <string>
#include <map>
#include <optional>
#include <boost/container/small_vector.hpp>

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
    flecs::entity entity{};
};

struct TileMeta {
    TileType type{};
    TileLayer layer{};
    std::string friendly_name{};
    std::optional<std::string> prefab_name{};
    // TODO: Use asset manager for this
    std::string spritesheet_name{};
    glm::ivec2 sprite_origin{};
    glm::ivec2 sprite_size{};
};

std::map<TileType, TileMeta> LoadTileMetas();

struct TileCoordNeighbors {
    using NeighborList = boost::container::small_vector<TileCoord, 8>;

    std::optional<TileCoord> north, northeast, east, southeast,
        south, southwest, west, northwest;

    NeighborList ToList() const;
};