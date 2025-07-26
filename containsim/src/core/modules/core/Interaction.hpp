#pragma once

#include <common/Ticks.hpp>
#include <flecs.h>

namespace core {
    struct Interactable {
        TickDuration time_needed{};
        TickDuration cooldown{};
    };

    struct InteractingWith { };
    struct InteractionProgress {
        TickDuration time_elapsed{};
        TickDuration time_needed{};
    };

    // These two events are for the actor doing the interaction
    struct InteractionerCompleteEvent {
        flecs::entity interactionee;
    };
    struct InteractionerFailedEvent {
        flecs::entity interactionee;
    };

    // These two events are for the object being interacted upon
    struct InteractioneeCompleteEvent {
        flecs::entity interactioner;
    };
    struct InteractioneeFailedEvent {
        flecs::entity interactioner;
    };

    void SetupInteractionSystems(flecs::world& world);
}