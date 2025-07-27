#include <modules/core/Interaction.hpp>
#include <modules/core/Cooldown.hpp>
#include <modules/core/Time.hpp>

using namespace core;

void core::SetupInteractionSystems(flecs::world& world) {
    world.component<Interactable>();
    world.component<InteractionProgress>();
    
    world.component<InteractingWith>();

    world.observer()
        .with<InteractingWith>().second(flecs::Wildcard)
        .event(flecs::OnAdd)
        .each([](flecs::iter& it, size_t idx) {
            flecs::entity interactioner = it.entity(idx);
            flecs::entity interactionee = it.pair(0).second();

            if (interactioner.has<InteractionProgress>()) {
                throw std::runtime_error("Cannot interact with multiple things at once");
            }

            interactioner.set(InteractionProgress{
                .time_elapsed = {},
                .time_needed = interactionee.get<Interactable>().time_needed
            });
        });

    world.system<InteractionProgress>("InteractionProgress")
        .with<InteractingWith>(flecs::Wildcard)
        .tick_source(world.lookup("core::Core::TickSource"))
        .each([](flecs::iter& it, size_t idx, InteractionProgress& p) {
            p.time_elapsed++;

            if (p.time_elapsed < p.time_needed)
                return;

            flecs::entity interactioner = it.entity(idx);
            flecs::entity interactionee = it.pair(1).second();

            interactioner.remove<InteractingWith>(interactionee);
        })
        .add<core::DependsOnTicks>();

    world.observer<InteractionProgress>()
        .term_at(0).filter()
        .with<InteractingWith>().second(flecs::Wildcard)
        .event(flecs::OnRemove)
        .each([](flecs::iter& it, size_t idx, InteractionProgress& p) {
            flecs::entity interactioner = it.entity(idx);
            flecs::entity interactionee = it.pair(1).second();

            if (p.time_needed >= p.time_elapsed) {
                interactioner.enqueue<InteractionerCompleteEvent>({interactionee});
                interactionee.enqueue<InteractioneeCompleteEvent>({interactioner});

                TickDuration cooldown = interactionee.get<Interactable>().cooldown;
                interactionee.set(Cooldown{.time_remaining = cooldown});
            } else {
                interactioner.enqueue<InteractionerFailedEvent>({interactionee});
                interactionee.enqueue<InteractioneeFailedEvent>({interactioner});
            }

            interactioner.remove<InteractionProgress>();
        });
}