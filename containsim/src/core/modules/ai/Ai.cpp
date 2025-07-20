#include <modules/ai/Ai.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/core/Core.hpp>

using namespace ai;

Ai::Ai(flecs::world& world) {
    world.import<core::Core>();
    world.import<physics::Physics>();
    
    world.component<Performer>();

    world.system<const Performer>("AiInputCollisionBegin")
        .with<physics::CollisionBegan>(flecs::Wildcard)
        .each([](flecs::iter& it, size_t, const Performer& p) {
            if (p.performance == nullptr)
                return;

            p.performance->GetFsm().InsertEvent("CollisionBegan", it.pair(1).second());
        });

    world.system<const Performer>("AiInputTick")
        .tick_source(world.lookup("core::Core::TickSource"))
        .kind(flecs::OnUpdate)
        .each([](const Performer& p) {
            if (p.performance == nullptr)
                return;

            p.performance->GetFsm().InsertEvent("Tick");
        });

    world.observer()
        .with<Performer>()
        .event(flecs::OnAdd)
        .yield_existing()
        .each([](flecs::entity e) {
            e.observe([](flecs::entity e, const core::InteractionerCompleteEvent& i) {
                e.get<Performer>().performance->GetFsm().InsertEvent("InteractionComplete", i.interactionee);
            });

            e.observe([](flecs::entity e, const core::InteractionerFailedEvent& i) {
                e.get<Performer>().performance->GetFsm().InsertEvent("InteractionFailed", i.interactionee);
            });
        });
}