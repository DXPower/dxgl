#include "Block.hpp"
#include <dxgl/Uniform.hpp>
#include <dxgl/Ubo.hpp>

#include <cstddef>
#include <memory>
#include <glad/glad.h>
#include <type_traits>

namespace {
        
    glm::vec4 GetBlockColor(BlockType type) {
        using enum BlockType;

        switch (type) {
            case GRASS_TOP: return {0, 1, 0, 1};
            case STONE: return {0.7, 0.7, 0.7, 1};
            case DIRT: return {0.5, 0.25, 0.25, 1};
            case GRASS: return {0, .7, 0, 1};
            case WOOD: return {0.6, 0.5, 0.33, 1};
            case LAVA: return {1, 0, 0, 1};
        }
    }

    struct ChunkDrawInfo {
        dxgl::Program program{};
        dxgl::Vao vao{};
        dxgl::Vbo vbo{};
    };

    struct ChunkVbo {
        std::array<glm::vec2, 4> block_corners{};
        std::array<glm::vec4, Chunk::blocks_per_chunk> colors{};
    };

    static_assert(std::is_trivially_copyable_v<ChunkVbo>);

    std::optional<ChunkDrawInfo> chunk_draw{};
}

void Chunk::InitDraw(const GlobalState& global_state) {
    using namespace dxgl;

    Program program = ProgramBuilder()
            .Vert("shaders/chunk.vert")
            .Frag("shaders/chunk.frag")
            .Link();

    ChunkVbo vbo_data{};
    vbo_data.block_corners = {{
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    }};

    Vbo vbo{};
    vbo.Upload(vbo_data, BufferUsage::Dynamic);

    Vao vao{};
    VaoAttribBuilder()
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
            )
        )
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(4)
                .PerInstance()
            )
            .Offset(offsetof(ChunkVbo, colors))
        )
        .Apply(vao, vbo);

    dxgl::Uniform::Set(program, "block_size", glm::vec2(100, 100));
    dxgl::Uniform::Set(program, "chunk_size", glm::ivec2(chunk_width, chunk_height));

    chunk_draw = ChunkDrawInfo{std::move(program), std::move(vao), std::move(vbo)};

    global_state.ubo_manager.BindUniformLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        program, 
        "camera"
    );
}

void Chunk::Render() const {
    decltype(std::declval<ChunkVbo>().colors) vbo_data{};

    for (std::size_t i = 0; i < blocks.size(); i++) {
        vbo_data[i] = GetBlockColor(blocks[i].type);
    }

    chunk_draw->vbo.Update(vbo_data, offsetof(ChunkVbo, colors));

    chunk_draw->vao.Use();
    chunk_draw->program.Use();

    dxgl::Uniform::Set(chunk_draw->program, "chunk_origin", glm::vec2(0, 0));

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, blocks_per_chunk);
}
