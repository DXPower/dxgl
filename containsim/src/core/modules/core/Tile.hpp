#pragma once

#include <modules/core/TileCoord.hpp>
#include <modules/core/Room.hpp>
#include <glm/vec2.hpp>
#include <flecs.h>
#include <string>
#include <map>
#include <optional>
#include <generator>

namespace core {
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
    RoomId room{};
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

struct TileSelection {
    TileCoord start{}; // Inclusive
    TileCoord end{}; // Inclusive

    bool Contains(const TileCoord& coord) const;
    std::generator<TileCoord> Iterate() const;

    int Area() const;
};
}