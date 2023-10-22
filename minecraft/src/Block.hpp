#pragma once

#include "GlobalState.hpp"

#include <dxgl/Shader.hpp>
#include <dxgl/Vao.hpp>

#include <array>
#include <span>
#include <glm/glm.hpp>

enum class BlockType {
    AIR,
    GRASS_TOP,
    STONE,
    DIRT,
    GRASS,
    WOOD,
    LAVA
};

struct Block {
    BlockType type{};
    glm::ivec2 rel_coord{};
};

struct Chunk {
    static constexpr glm::vec2 block_size = {100, 100};
    static constexpr std::size_t chunk_width = 3;
    static constexpr std::size_t chunk_height = 2;
    static constexpr std::size_t blocks_per_chunk = chunk_width * chunk_height;

    std::array<Block, blocks_per_chunk> blocks{};
    glm::vec2 origin{};
    
    Chunk();
    void Render() const;

    glm::vec2 GetCoordPos(glm::ivec2 rel_coord) const;

    static void InitDraw(const GlobalState& global_state);
    static std::array<glm::ivec2, blocks_per_chunk> GetBlockCoords();
};
