#pragma once

#include <modules/core/TileCoord.hpp>
#include <modules/core/Room.hpp>
#include <glm/vec2.hpp>
#include <flecs.h>
#include <generator>

namespace core {

using TileType = int;
constexpr TileType NothingTile = 0;

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

struct TileSelection {
    TileCoord start{}; // Inclusive
    TileCoord end{}; // Inclusive

    bool Contains(const TileCoord& coord) const;
    std::generator<TileCoord> Iterate() const;

    int Area() const;
};
}