#include <modules/ui/Ui.hpp>
#include <modules/ui/InputStateBinding.hpp>
#include <modules/ui/TilesBinding.hpp>
#include <modules/ui/EconomyBinding.hpp>
#include <modules/ui/BuildPanel.hpp>
#include <modules/ui/RoomPanel.hpp>
#include <modules/ui/RoomTypesBinding.hpp>
#include <modules/application/Application.hpp>
#include <modules/core/Core.hpp>

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ElementDocument.h>
#include <dxgl/Screenbuffer.hpp>

using namespace ui;

Ui::Ui(flecs::world& world) {
    world.import<application::Application>();
    world.import<core::Core>();

    auto& event_manager = world.get_mut<application::EventManager>();
    
    world.component<RmlEventManager>().add(flecs::Sparse);
    world.emplace<RmlEventManager>(event_manager);

    auto& ui_event_manager = world.get_mut<RmlEventManager>();
    Rml::Factory::RegisterEventListenerInstancer(&ui_event_manager);

    const auto& context_handle = world.query<application::RmlMainContext>()
        .first().get<const application::RmlContextHandle>();

    world.component<InputStateBinding>().add(flecs::Sparse);
    world.emplace<InputStateBinding>(
        InputStates::IdleMode,
        *context_handle.context,
        event_manager
    );

    world.component<TilesBinding>().add(flecs::Sparse);
    world.emplace<ui::TilesBinding>(
        *context_handle.context,
        world
    );

    world.component<EconomyBinding>().add(flecs::Sparse);
    world.emplace<EconomyBinding>(
        *context_handle.context,
        world
    );

    world.component<RoomTypesBinding>().add(flecs::Sparse);
    world.emplace<RoomTypesBinding>(
        *context_handle.context,
        world
    );

    // Load the document
    auto* document = context_handle.context->LoadDocument("res/ui/game/test.rml");

    if (document == nullptr) {
        throw std::runtime_error("Failed to load RmlUi document");
    }

    document->Show();

    // TODO: Componentize the Panels
    world.component<BuildPanel>().add(flecs::Sparse);
    world.emplace<BuildPanel>(event_manager);

    world.component<RoomPanel>().add(flecs::Sparse);;
    world.emplace<RoomPanel>(event_manager, world.get<core::RoomTypeMetas>());

    // UI render phase, which comes after the world is rendered
    // (This is why this system is in the UI module, not the application module)
    flecs::entity world_render_e = world.lookup("rendering::Rendering::WorldRenderPhase");
    flecs::entity ui_render_e = world.entity("UiRenderPhase")
        .add(flecs::Phase)
        .add(flecs::DependsOn, world_render_e);

    // Mark the final presentation as dependent on this phase
    world.lookup("application::Application::FinalPresentPhase")
        .add(flecs::DependsOn, ui_render_e);

    // May need custom pipeline if we plan to have multiple UI contexts
    world.system<application::UiEnv, application::RmlContextHandle>("RenderUi")
        .term_at<application::UiEnv>().singleton()
        .kind(ui_render_e)
        .each([](application::UiEnv& ui_env, const application::RmlContextHandle& context_handle) {
            dxgl::Screenbuffer::Unuse();

            ui_env.GetRenderInterface().BeginFrame();
            context_handle.context->Render();
            ui_env.GetRenderInterface().EndFrame();
        });
}
