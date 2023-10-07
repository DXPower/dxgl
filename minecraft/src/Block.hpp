#pragma once

#include "GlobalState.hpp"

#include <dxgl/Shader.hpp>
#include <dxgl/Vao.hpp>

#include <array>
#include <span>
#include <glm/glm.hpp>

enum class BlockType {
    GRASS_TOP,
    STONE,
    DIRT,
    GRASS,
    WOOD,
    LAVA
};

struct Block {
    BlockType type{};
    glm::vec<2, int64_t> position{};
};

struct Chunk {
    static constexpr std::size_t chunk_width = 3;
    static constexpr std::size_t chunk_height = 2;
    static constexpr std::size_t blocks_per_chunk = chunk_width * chunk_height;
    std::array<Block, blocks_per_chunk> blocks{};

    void Render() const;

    static void InitDraw(const GlobalState& global_state);
    static std::array<glm::vec<2, int>, blocks_per_chunk> GetBlockCoords();
};
