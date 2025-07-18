#include <modules/input/InputState.hpp>
#include <modules/input/BuildInput.hpp>
#include <modules/input/RoomInput.hpp>

using namespace input;

void input::SetupInputState(flecs::world& world) {
    const auto& camera = world.get<rendering::Camera>();
    const auto& tile_grid = world.get<core::TileGrid>();
    auto& event_manager = world.get_mut<application::EventManager>();

    world.component<MeceFsm>().add(flecs::Sparse);

    auto input_state_e = world.entity("InputState")
        .emplace<MeceFsm>("InputState");

    auto& input_state_mece = input_state_e.get_mut<MeceFsm>();

    input_state_mece.AddSubFsm(std::make_unique<BuildInput>(
        event_manager, camera, tile_grid
    ));

    input_state_mece.AddSubFsm(std::make_unique<RoomInput>(
        event_manager, camera, tile_grid
    ));
}
