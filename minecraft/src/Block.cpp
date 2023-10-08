#include "Block.hpp"
#include <dxgl/Texture.hpp>
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
            case AIR: return {0, 0, 0, 0}; break;
            case GRASS_TOP: return {0, 1, 0, 1};
            case STONE: return {0.7, 0.7, 0.7, 1};
            case DIRT: return {0.5, 0.25, 0.25, 1};
            case GRASS: return {0, .7, 0, 1};
            case WOOD: return {0.6, 0.5, 0.33, 1};
            case LAVA: return {1, 0, 0, 1};
        }
    }

    glm::vec2 GetBlockSpriteOrigin(BlockType type) {
        using enum BlockType;

        glm::vec2 coord;
        switch (type) {
            case AIR: coord = {23, 33}; break;
            case GRASS_TOP: coord = {0, 0}; break;
            case STONE: coord = {1, 0}; break;
            case DIRT: coord = {2, 0}; break;
            case GRASS: coord = {3, 0}; break;
            case WOOD: coord = {4, 0}; break;
            case LAVA: coord = {11, 18}; break;
        }

        return coord * 16.f;
    }

    glm::vec2 GetGlSpriteOrigin(BlockType type, glm::vec2 sheet_size) {
        auto sprite_origin = GetBlockSpriteOrigin(type);

        // Move the origin to the bottom-left (for OGL texture coordinates)
        sprite_origin.y += 16.f;

        // Normalize to [0, 1]
        sprite_origin /= sheet_size;
        
        // Flip y-axis for OGL texture coordinates
        sprite_origin.y = sheet_size.y - sprite_origin.y;

        return sprite_origin;
    };

    struct ChunkDrawInfo {
        dxgl::Program program{};
        dxgl::Vao vao{};
        dxgl::Vbo vbo{};
        dxgl::Texture spritesheet{};
    };

    struct ChunkVbo {
        struct VertData {
            glm::vec2 position{};
            glm::vec2 tex_pos{};
        };

        struct BlockData {
            glm::vec4 color{};
            glm::vec2 sprite_origin{};
        };

        std::array<VertData, 4> block_verts{};
        std::array<BlockData, Chunk::blocks_per_chunk> block_data{};
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
    vbo_data.block_verts = {{
    //  Pos      Tex
        {{0, 0}, {0.075, 0.925}},
        {{1, 0}, {0.925, 0.925}},
        {{1, 1}, {0.925, 0.075}},
        {{0, 1}, {0.075, 0.075}} 
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
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
                .PerInstance()
            )
            .Offset(offsetof(ChunkVbo, block_data))
        )
        .Apply(vao, vbo);

    dxgl::Uniform::Set(program, "block_size", block_size);
    dxgl::Uniform::Set(program, "chunk_size", glm::ivec2(chunk_width, chunk_height));
    
    dxgl::Texture spritesheet = dxgl::LoadTextureFromFile("res/img/blocks.png");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const glm::vec2 sprite_size = 16.f / (glm::vec2) spritesheet.GetSize();
    dxgl::Uniform::Set(program, "block_sprite_size", sprite_size);
    dxgl::Uniform::Set(program, "spritesheet", 0);

    chunk_draw = ChunkDrawInfo{
        std::move(program),
        std::move(vao),
        std::move(vbo),
        std::move(spritesheet)
    };

    global_state.ubo_manager.BindUniformLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        program, 
        "camera"
    );
}

void Chunk::Render() const {
    decltype(std::declval<ChunkVbo>().block_data) vbo_data{};

    const glm::vec2 spritesheet_size = chunk_draw->spritesheet.GetSize();

    for (std::size_t i = 0; i < blocks.size(); i++) {
        vbo_data[i].color = GetBlockColor(blocks[i].type);
        vbo_data[i].sprite_origin = GetGlSpriteOrigin(blocks[i].type, spritesheet_size);
    }

    chunk_draw->vbo.Update(vbo_data, offsetof(ChunkVbo, block_data));

    chunk_draw->vao.Use();
    chunk_draw->program.Use();
    chunk_draw->spritesheet.Use(0);

    dxgl::Uniform::Set(chunk_draw->program, "chunk_origin", glm::vec2(0, 0));

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, blocks_per_chunk);
}
