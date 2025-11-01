#include <modules/rendering/TileGridRenderer.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/UboLocs.hpp>
#include <modules/core/Tile.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/core/TileTypeMeta.hpp>

#include <dxgl/Uniform.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Vbo.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Ubo.hpp>

#include <array>
#include <fstream>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <magic_enum/magic_enum_containers.hpp>
#include <nlohmann/json.hpp>

using namespace rendering;
using namespace core;

namespace {
    struct PerInstanceData {
        glm::mat3 world{}; // center is origin
        glm::mat3 tex{}; // top left is origin
    };

    struct VertexData {
        glm::vec2 local_pos{};
        glm::vec2 tex_pos{};
    };

    constexpr std::array<VertexData, 4> quad_vbo_data{
               //  Pos            Tex
        VertexData{{-.5f, -.5f},  {0.05f, 0.95f}},
        VertexData{{ .5f, -.5f},  {0.95f, 0.95f}},
        VertexData{{ .5f,  .5f},  {0.95f, 0.05f}},
        VertexData{{-.5f,  .5f},  {0.05f, 0.05f}} 
    };

    struct TileSpriteData {
        dxgl::TextureView spritesheet{};
        glm::ivec2 origin{};
        glm::ivec2 size{};
    };

    struct TileGridRendererData {
        std::unordered_map<TileType, TileSpriteData> tile_sprites{};

        // Draw data
        mutable magic_enum::containers::array<TileLayer, std::optional<dxgl::Draw>> cached_draws{};
        
        mutable dxgl::Program program{};
        dxgl::Vbo quad_vbo{};

        TileGridRendererData(const core::TileGrid& tiles, const core::TileTypeMetas& metas, const dxgl::UboBindingManager& ubo_manager) {
            tiles.tile_update_signal.connect<&TileGridRendererData::OnTileUpdate>(this);
            
            program = dxgl::ProgramBuilder()
                .Vert("shaders/sprite.vert")
                .Frag("shaders/sprite.frag")
                .Link();

            dxgl::Uniform::Set(program, "spritesheet", 0);
            
            ubo_manager.BindUniformLocation(
                static_cast<std::size_t>(UboLocs::Camera), 
                program, 
                "camera"
            );

            quad_vbo.Upload(quad_vbo_data, dxgl::BufferUsage::Static);

            LoadTileSpriteData(metas);
        }

        void LoadTileSpriteData(const core::TileTypeMetas& metas) {
            for (const auto& [id, meta] : metas.GetMetas()) {
                auto& sprite_data = tile_sprites[id];
                sprite_data.spritesheet = meta.spritesheet;
                sprite_data.origin = meta.sprite_origin;
                sprite_data.size = meta.sprite_size;
            }
        }

        std::vector<PerInstanceData> BuildInstanceData(const core::TileGrid& tiles, TileLayer layer) const {
            std::vector<PerInstanceData> instances{};

            const auto tile_world_size = tiles.GetTileWorldSize();

            // Build the instanced sprite data for each tile
            for (const auto& col : tiles.GetUnderlyingGrid()[layer]) {
                for (const Tile& tile : col) {
                    if (tile.data.type == NothingTile)
                        continue;
                        
                    const glm::vec2 world_pos = (glm::vec2) tile.coord * tile_world_size;

                    auto world_mat = glm::mat3(1);
                    world_mat = glm::translate(world_mat, world_pos);
                    world_mat = glm::scale(world_mat, tile_world_size);

                    const auto& sprite_data = tile_sprites.at(tile.data.type);
                    const glm::vec2 tex_size = sprite_data.spritesheet->GetSize();

                    // Need to flip the Y coordinate for OpenGL
                    glm::vec2 pos_mod = { 
                        sprite_data.origin.x,
                        tex_size.y - (sprite_data.origin.y + sprite_data.size.y)
                    };

                    auto tex_mat = glm::mat3(1);
                    tex_mat = glm::translate(tex_mat, pos_mod / tex_size);
                    tex_mat = glm::scale(tex_mat, (glm::vec2) sprite_data.size / tex_size);

                    instances.push_back({
                        .world = world_mat,
                        .tex = tex_mat
                    });
                }
            }

            return instances;
        }

        dxgl::Draw BuildDraw(const core::TileGrid& tiles, TileLayer layer) const {
            auto instances = BuildInstanceData(tiles, layer);

            // Make the draw to be queued using the generated instance data
            dxgl::Draw draw{};
            draw.program = program;
            draw.prim_type = dxgl::PrimType::TriangleFan;
            draw.num_indices = 4;
            draw.num_instances = (uint32_t) instances.size();
            draw.vao_storage.emplace();
            draw.vbo_storage.emplace_back().Upload(instances, dxgl::BufferUsage::Static);
            // TODO: Don't assume all tiles have the same spritesheet
            draw.textures.push_back(tile_sprites.begin()->second.spritesheet);

            using namespace dxgl;

            VaoAttribBuilder()
                .Group(AttribGroup()
                    .Vbo(quad_vbo)
                    .Attrib(Attribute()
                        .Type(AttribType::Float)
                        .Components(2)
                        .Multiply(2)
                    )
                )
                .Group(AttribGroup()
                    .Vbo(draw.vbo_storage.back())
                    .Attrib(Attribute()
                        .Type(AttribType::Float)
                        .Matrix(3, 3)
                        .PerInstance()
                    )
                    .Attrib(Attribute()
                        .Type(AttribType::Float)
                        .Matrix(3, 3)
                        .PerInstance()
                    )
                )
                .Apply(*draw.vao_storage);

            return draw;
        }

        void OnTileUpdate(const core::TileGrid&, const Tile& tile) {
            cached_draws[tile.layer].reset();
        }
    };

    void Render(const TileGridRendererData& data, const core::TileGrid& tiles, DrawQueues& draws) {
        for (auto layer : magic_enum::enum_values<TileLayer>()) {
            if (!data.cached_draws[layer].has_value())
                data.cached_draws[layer] = data.BuildDraw(tiles, layer);

            magic_enum::containers::array<TileLayer, RenderLayer> render_layer_map{};
            render_layer_map[TileLayer::Subterranean] = RenderLayer::Floors;
            render_layer_map[TileLayer::Ground] = RenderLayer::Floors;
            render_layer_map[TileLayer::Walls] = RenderLayer::Objects;
            render_layer_map[TileLayer::Ceiling] = RenderLayer::Ceilings;

            draws.QueueViewedDraw(render_layer_map[layer], *data.cached_draws[layer]);
        }
    }
}

void rendering::TileGridRendererSystem(flecs::world& world) {
    const auto& ubos = world.get<dxgl::UboBindingManager>();
    const auto& tiles = world.get<core::TileGrid>();
    const auto& metas = world.get<core::TileTypeMetas>();

    world.component<TileGridRendererData>();
    world.emplace<TileGridRendererData>(tiles, metas, ubos);

    world.system<const TileGridRendererData, const core::TileGrid, DrawQueues>()
        .term_at<TileGridRendererData>().singleton()
        .term_at<core::TileGrid>().singleton()
        .term_at<DrawQueues>().singleton()
        .kind(flecs::OnStore)
        .each(&Render);
}