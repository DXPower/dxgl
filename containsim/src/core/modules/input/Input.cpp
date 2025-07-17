#include <modules/input/Input.hpp>
#include <modules/application/Application.hpp>
#include <modules/core/Core.hpp>
#include <modules/rendering/Rendering.hpp>

using namespace input;

Input::Input(flecs::world& world) {
    world.import<application::Application>();
    world.import<core::Core>();
    world.import<rendering::Rendering>();

    const auto& main_window = world.query<application::MainWindow>()
        .first().get<dxgl::Window>();

    world.component<InputHandler>().add(flecs::Sparse);
    world.emplace<InputHandler>(main_window);
    auto& input_handler = world.get_mut<InputHandler>();

    world.component<UiActionReceiver>().add(flecs::Sparse);
    world.emplace<UiActionReceiver>(world);
    auto& ui_actions = world.get_mut<UiActionReceiver>();
    
    chain::Connect(input_handler.actions_out, ui_actions);

    // world.component<BuildInput>().add(flecs::Sparse);
    // world.emplace<BuildInput>(event_manager, camera, tile_grid);
    // auto& build_input = world.get_mut<BuildInput>();

    // world.component<RoomInput>().add(flecs::Sparse);
    // world.emplace<RoomInput>(event_manager, camera, tile_grid);
    // auto& room_input = world.get_mut<RoomInput>();

    SetupInputState(world);

    auto& input_state = world.lookup("input::Input::InputState")
        .get_mut<MeceFsm>();

    world.component<ActionRouter>().add(flecs::Sparse);
    world.emplace<ActionRouter>(main_window, input_state);

    auto& action_router = world.get_mut<ActionRouter>();
    chain::Connect(ui_actions.uncaptured_actions, action_router);
    // world.component<InputState>().add(flecs::Sparse);
    // world.emplace<InputState>(event_manager, build_input, room_input);
    // auto& input_state = world.get_mut<InputState>();

    // chain::Connect(action_router.game_action_receiver, input_state);
    // chain::Connect(action_router.offscreen_action_receiver, input_state);

    // chain::Connect(input_state.build_actions, build_input);
    // chain::Connect(input_state.room_actions, room_input);
}