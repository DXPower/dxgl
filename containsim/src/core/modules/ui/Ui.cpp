#include <modules/ui/Ui.hpp>
#include <modules/ui/InputStateBinding.hpp>
#include <modules/ui/TilesBinding.hpp>
#include <modules/ui/BuildPanel.hpp>
#include <modules/ui/RoomPanel.hpp>
#include <modules/application/Application.hpp>

#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ElementDocument.h>

using namespace ui;

Ui::Ui(flecs::world& world) {
    world.import<application::Application>();

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
        *context_handle.context
    );

    // Load the document
    auto* document = context_handle.context->LoadDocument("res/ui/game/test.rml");

    if (document == nullptr) {
        throw std::runtime_error("Failed to load RmlUi document");
    }

    document->Show();

    // ui::BuildPanel build_panel{event_manager, *document};
    // ui::RoomPanel room_panel{event_manager, *document};

    // TODO: Componentize the Panels
    world.component<BuildPanel>().add(flecs::Sparse);
    world.emplace<BuildPanel>(event_manager);

    world.component<RoomPanel>().add(flecs::Sparse);;
    world.emplace<RoomPanel>(event_manager);
}
