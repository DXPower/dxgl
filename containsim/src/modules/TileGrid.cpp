#include <modules/TileGrid.hpp>
#include <common/GlobalData.hpp>
#include <common/Rendering.hpp>
#include <common/Tile.hpp>

#include <dxgl/Uniform.hpp>
#include <dxgl/Texture.hpp>

#include <array>
#include <fstream>
#include <vector>
#include <boost/multi_array.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <magic_enum_containers.hpp>
#include <nlohmann/json.hpp>

using namespace TileGrid;

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
}

class Module::Pimpl {
public:
    // Tile data
    boost::multi_array<Tile, 2> tiles{};
    magic_enum::containers::array<TileType, TileSpriteData> tile_sprites{};
    glm::vec2 tile_world_size{};

    // Draw data
    mutable std::optional<dxgl::Draw> cached_draw{};
    
    mutable dxgl::Program program{};
    dxgl::Vbo quad_vbo{};
    dxgl::Texture spritesheet{};

    Pimpl(const GlobalState& global_state) {
        spritesheet = dxgl::LoadTextureFromFile("res/img/tiles.png");
        spritesheet.SetFilterMode(dxgl::FilterMode::Nearest);
        
        program = dxgl::ProgramBuilder()
            .Vert("shaders/sprite.vert")
            .Frag("shaders/sprite.frag")
            .Link();

        dxgl::Uniform::Set(program, "spritesheet", 0);
        
        global_state.ubo_manager.BindUniformLocation(
            static_cast<std::size_t>(UboLocs::Camera), 
            program, 
            "camera"
        );

        quad_vbo.Upload(quad_vbo_data, dxgl::BufferUsage::Static);

        LoadTileSpriteData();
        (void) 0;
    }

    void LoadTileSpriteData() {
        std::ifstream json_file("res/spritesheets.json");
        nlohmann::json json_data = nlohmann::json::parse(json_file);

        for (const auto& sheet_json : json_data["sheets"]) {
            if (sheet_json["name"] == "tiles") {
                for (const auto& sprite_json : sheet_json["sprites"]) {
                    auto name = sprite_json["name"].template get<std::string>();
                    auto type = magic_enum::enum_cast<TileType>(name).value();

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

    std::vector<PerInstanceData> BuildInstanceData() const {
        std::vector<PerInstanceData> instances{};

        // Build the instanced sprite data for each tile
        for (auto col : tiles) {
            for (const Tile& tile : col) {
                const glm::vec2 world_pos = (glm::vec2) tile.coord * tile_world_size;

                auto world_mat = glm::mat3(1);
                world_mat = glm::translate(world_mat, world_pos);
                world_mat = glm::scale(world_mat, tile_world_size);

                const auto& cutout = tile_sprites[tile.data.type];
                const glm::vec2 tex_size = spritesheet.GetSize();

                glm::vec2 pos_mod = { 
                    cutout.origin.x,
                    cutout.size.y - cutout.origin.y - cutout.size.y
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

    dxgl::Draw BuildDraw() const {
        auto instances = BuildInstanceData();

        // Make the draw to be queued using the generated instance data
        dxgl::Draw draw{};
        draw.program = program;
        draw.prim_type = dxgl::PrimType::TriangleFan;
        draw.num_indices = 4;
        draw.num_instances = instances.size();
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
};

void Module::PimplDeleter::operator()(Pimpl* p) const {
    delete p;
}

Module::Module(flecs::world& world) {
    // world.module<Module>("TileGrid");
    const auto& global_data = *world.get<GlobalData>();
    const auto& config = *global_data.config;
    const auto& global_state = *global_data.state;

    m_pimpl.reset(new Pimpl(global_state));
    
    m_pimpl->tile_world_size = config.tile_size;

    auto& tiles = m_pimpl->tiles;
    tiles.resize(boost::extents[config.map_size.x][config.map_size.y]);

    for (int x = 0; (std::size_t) x < tiles.size(); x++) {
        for (int y = 0; (std::size_t) y < tiles[x].size(); y++) {
            auto& tile = tiles[x][y];

            tile.coord = {x, y};
        }
    }
}

Module::Module(Module&& move) = default;
Module::~Module() = default;
Module& Module::operator=(Module&& move) = default;

void Module::SetTile(const glm::ivec2& coord, const TileData& data) {
    m_pimpl->tiles[coord.x][coord.y].data = data;

    // Reset the cached draw when we change the tile data
    m_pimpl->cached_draw.reset();
}

void Module::Render(DrawQueues& draws) const {
    if (!m_pimpl->cached_draw.has_value())
        m_pimpl->cached_draw = m_pimpl->BuildDraw();

    draws.QueueViewedDraw(RenderLayer::Background, *m_pimpl->cached_draw);
}