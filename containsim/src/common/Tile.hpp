#pragma once

#include <glm/vec2.hpp>

enum class TileType {
    Grass,
    Dirt,
    Tile
};

struct TileData {
    TileType type{};
};

struct Tile {
    glm::ivec2 coord{};
    TileData data{};
};