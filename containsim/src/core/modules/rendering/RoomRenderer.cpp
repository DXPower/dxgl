#include <modules/rendering/RoomRenderer.hpp>
#include <modules/rendering/UboLocs.hpp>
#include <modules/core/Core.hpp>
#include <modules/application/EventManager.hpp>

#include <dxgl/Uniform.hpp>
#include <dxgl/Texture.hpp>

#include <array>
#include <vector>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <magic_enum/magic_enum_containers.hpp>
#include <nlohmann/json.hpp>

using namespace rendering;
using namespace core;

namespace {
    struct VertexData {
        glm::vec2 world_pos{};
        glm::vec2 inner_normal{};
        float is_corner{}; // 0 = false, 1 = true
    };

    struct RoomRendererData {
        const core::RoomManager* room_manager{};

        // Draw data
        mutable std::unordered_map<RoomId, std::optional<dxgl::Draw>> cached_draws{};
        
        mutable dxgl::Program program{};

        RoomRendererData(const core::RoomManager& room_manager, const dxgl::UboBindingManager& ubo_manager, application::EventManager& em) {
            this->room_manager = &room_manager;

            em.GetOrRegisterSignal<RoomAdded>()
                .signal.connect<&RoomRendererData::OnRoomAdded>(this);
            em.GetOrRegisterSignal<RoomModified>()
                .signal.connect<&RoomRendererData::OnRoomModified>(this);
            em.GetOrRegisterSignal<RoomRemoved>()
                .signal.connect<&RoomRendererData::OnRoomRemoved>(this);

            program = dxgl::ProgramBuilder()
                .Vert("shaders/room.vert")
                .Geom("shaders/room.geom")
                .Frag("shaders/room.frag")
                .Link();

            
            ubo_manager.BindUniformLocation(
                static_cast<std::size_t>(UboLocs::Camera), 
                program, 
                "camera"
            );
        }

        std::array<VertexData, 2> GetBorderLine(
            const std::unordered_set<TileCoord>& room_tiles,
            TileCoord center,
            TileCoord check,
            std::array<TileCoord, 2> potential_edges) const {

            // Make the base line coords
            const auto& tile_grid = room_manager->GetTileGrid();
            const auto outer_normal = check - center;
            const auto line_offset = (tile_grid.GetTileWorldSize() / 2.f)
                * (glm::vec2)(outer_normal); // Get the direction the line should be shifted to the border
            
            auto vertices = std::array<VertexData, 2>{};
            vertices[0].world_pos = tile_grid.TileCoordToWorldPos(center) + line_offset;
            vertices[0].inner_normal = -outer_normal;
            vertices[1] = vertices[0];

            // Now shift each vertex to the other edges of the tile
            for (std::size_t i = 0; i < potential_edges.size(); i++) {
                auto edge = potential_edges[i];
                auto& vertex = vertices[i];

                vertex.world_pos += (tile_grid.GetTileWorldSize() / 2.f)
                    * (glm::vec2)(edge - center);

                // Check if the edge is a corner
                vertex.is_corner = room_tiles.contains(edge) ? 1.f : 0.f;
            }

            return vertices;
        }

        std::vector<VertexData> BuildRoomBorders(const Room& room) const {
            const auto& tiles = room.GetTiles();
            std::vector<VertexData> vertices{};

            auto CheckTile = [&](TileCoord center, TileCoord check, std::array<TileCoord, 2> potential_corners) {
                if (!tiles.contains(check)) {
                    vertices.append_range(
                        GetBorderLine(tiles, center, check, potential_corners)
                    );
                }
            };

            for (const auto& tile : tiles) {
                auto neighbors = GetBoundlessTileCoordNeighbors(tile);

                CheckTile(tile, *neighbors.north, {*neighbors.east, *neighbors.west});
                CheckTile(tile, *neighbors.south, {*neighbors.east, *neighbors.west});
                CheckTile(tile, *neighbors.east, {*neighbors.north, *neighbors.south});
                CheckTile(tile, *neighbors.west, {*neighbors.north, *neighbors.south});
            }

            return vertices;
        }

        dxgl::Draw BuildDraw(const RoomManager& room_manager, const RoomTypeMetas& room_types, const Room& room) const {
            auto vertex_data = BuildRoomBorders(room);

            // Make the draw to be queued using the generated instance data
            dxgl::Draw draw{};
            draw.program = program;
            draw.prim_type = dxgl::PrimType::Line;
            draw.num_indices = vertex_data.size() * 2;
            draw.vao_storage.emplace();
            draw.vbo_storage.emplace_back().Upload(vertex_data, dxgl::BufferUsage::Static);
            draw.options.wireframe = true;
            draw.options.line_width = 4.f;

            const auto& type_meta = room_types.Get(room.GetType());

            if (type_meta == nullptr) {
                throw std::runtime_error(std::format("Could not find meta information for room type {}", room.GetType()));
            }

            const glm::vec4 color = glm::vec4(type_meta->color, 1.f);
            const float ombre_width = room_manager.GetTileGrid().GetTileWorldSize().x / 5.f;

            draw.uniform_applicator = [color, ombre_width](dxgl::ProgramRef program) {
                dxgl::Uniform::Set(*program, "color", color);
                dxgl::Uniform::Set(*program, "ombre_width", ombre_width);
            };

            using namespace dxgl;

            VaoAttribBuilder()
                .Group(AttribGroup()
                    .Vbo(draw.vbo_storage.back())
                    .Attrib(Attribute()
                        .Type(AttribType::Float)
                        .Components(2)
                        .Multiply(2)
                    )
                    .Attrib(Attribute()
                        .Type(AttribType::Float)
                        .Components(1)
                    )
                )
                .Apply(*draw.vao_storage);

            return draw;
        }

        void OnRoomAdded(const RoomAdded& e) {
            cached_draws[e.room->GetId()] = std::nullopt;
        }

        void OnRoomModified(const RoomModified& e) {
            cached_draws[e.room->GetId()] = std::nullopt;
        }

        void OnRoomRemoved(const RoomRemoved& e) {
            cached_draws.erase(e.id);
        }
    };

    void Render(const RoomRendererData& data, const RoomTypeMetas& room_type_metas, DrawQueues& draws) {
        for (auto& [room_id, draw] : data.cached_draws) {
            if (!draw.has_value()) {
                draw = data.BuildDraw(*data.room_manager, room_type_metas, *data.room_manager->GetRoom(room_id));
            }
            
            draws.QueueViewedDraw(RenderLayer::Objects, *draw);
        }
    }
}

void rendering::RoomRendererSystem(flecs::world& world) {
    world.import<core::Core>();

    const auto& room_manager = world.get<core::RoomManager>();
    const auto& ubos = world.get<dxgl::UboBindingManager>();
    auto& event_manager = world.get_mut<application::EventManager>();

    world.component<RoomRendererData>();
    world.emplace<RoomRendererData>(room_manager, ubos, event_manager);

    world.system<const RoomRendererData, const RoomTypeMetas, DrawQueues>()
        .term_at<RoomRendererData>().singleton()
        .term_at<RoomTypeMetas>().singleton()
        .term_at<DrawQueues>().singleton()
        .each(&Render);
}