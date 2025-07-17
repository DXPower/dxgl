#include <modules/rendering/TileGridRenderer.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/UboLocs.hpp>
#include <modules/core/Tile.hpp>
#include <modules/core/TileGrid.hpp>

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
        glm::ivec2 origin{};
        glm::ivec2 size{};
    };

    struct TileGridRendererData {
        magic_enum::containers::array<TileType, TileSpriteData> tile_sprites{};

        // Draw data
        mutable magic_enum::containers::array<TileLayer, std::optional<dxgl::Draw>> cached_draws{};
        
        mutable dxgl::Program program{};
        dxgl::Vbo quad_vbo{};
        dxgl::Texture spritesheet{};

        TileGridRendererData(const core::TileGrid& tiles, const dxgl::UboBindingManager& ubo_manager) {
            tiles.tile_update_signal.connect<&TileGridRendererData::OnTileUpdate>(this);
            
            spritesheet = dxgl::LoadTextureFromFile("res/img/tiles.png");
            spritesheet.SetFilterMode(dxgl::FilterMode::Nearest);
            
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

            LoadTileSpriteData();
        }

        void LoadTileSpriteData() {
            std::ifstream json_file("res/spritesheets.json");
            nlohmann::json json_data = nlohmann::json::parse(json_file);

            for (const auto& sheet_json : json_data["sheets"]) {
                if (sheet_json["name"] == "tiles") {
                    for (const auto& sprite_json : sheet_json["sprites"]) {
                        auto name = sprite_json["name"].template get<std::string>();
                        TileType type{};
                        
                        if (auto casted = magic_enum::enum_cast<TileType>(name)) {
                            type = casted.value();
                        } else {
                            throw std::runtime_error("Invalid TileType in JSON file: " + name);
                        }

                        auto& sprite_data = tile_sprites[type];
                        
                        sprite_data.origin = {
                            sprite_json["x"].template get<int>(),
                            sprite_json["y"].template get<int>()
                        };

                        sprite_data.size = {
                            sprite_json["w"].template get<int>(),
                            sprite_json["h"].template get<int>()
                        };
                    }
                }
            }
        }

        std::vector<PerInstanceData> BuildInstanceData(const core::TileGrid& tiles, TileLayer layer) const {
            std::vector<PerInstanceData> instances{};

            const auto tile_world_size = tiles.GetTileWorldSize();

            // Build the instanced sprite data for each tile
            for (const auto& col : tiles.GetUnderlyingGrid()[layer]) {
                for (const Tile& tile : col) {
                    if (tile.data.type == TileType::Nothing)
                        continue;
                        
                    const glm::vec2 world_pos = (glm::vec2) tile.coord * tile_world_size;

                    auto world_mat = glm::mat3(1);
                    world_mat = glm::translate(world_mat, world_pos);
                    world_mat = glm::scale(world_mat, tile_world_size);

                    const auto& cutout = tile_sprites[tile.data.type];
                    const glm::vec2 tex_size = spritesheet.GetSize();

                    // Need to flip the Y coordinate for OpenGL
                    glm::vec2 pos_mod = { 
                        cutout.origin.x,
                        tex_size.y - (cutout.origin.y + cutout.size.y)
                    };

                    auto tex_mat = glm::mat3(1);
                    tex_mat = glm::translate(tex_mat, pos_mod / tex_size);
                    tex_mat = glm::scale(tex_mat, (glm::vec2) cutout.size / tex_size);

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
            draw.textures.push_back(spritesheet);

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

    world.component<TileGridRendererData>();
    world.emplace<TileGridRendererData>(tiles, ubos);

    world.system<const TileGridRendererData, const core::TileGrid, DrawQueues>()
        .term_at<TileGridRendererData>().singleton()
        .term_at<core::TileGrid>().singleton()
        .term_at<DrawQueues>().singleton()
        .kind(flecs::OnStore)
        .each(&Render);
}