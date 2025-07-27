#include <modules/ai/Ai.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/core/Core.hpp>

using namespace ai;

Ai::Ai(flecs::world& world) {
    world.import<core::Core>();
    world.import<physics::Physics>();
    
    world.component<Performer>();

    world.system<CurrentPerformance>("AiInputCollisionBegin")
        .with<physics::CollisionBegan>(flecs::Wildcard)
        .each([](flecs::iter& it, size_t, const CurrentPerformance& p) {
            if (p.performance == nullptr)
                return;

            p.performance->GetFsm().InsertEvent("CollisionBegan", it.pair(1).second());
        });

    world.system<const CurrentPerformance>("AiInputTick")
        .tick_source(world.lookup("core::Core::TickSource"))
        .kind(flecs::OnUpdate)
        .each([](const CurrentPerformance& p) {
            if (p.performance == nullptr)
                return;

            p.performance->GetFsm().InsertEvent("Tick");
        })
        .add<core::DependsOnTicks>();

    world.observer()
        .with<Performer>()
        .event(flecs::OnAdd)
        .yield_existing()
        .each([](flecs::entity e) {
            e.observe([](flecs::entity e, const core::InteractionerCompleteEvent& i) {
                const auto* p = e.try_get<CurrentPerformance>();

                if (p == nullptr)
                    return;

                p->performance->GetFsm().InsertEvent("InteractionComplete", i.interactionee);
            });

            e.observe([](flecs::entity e, const core::InteractionerFailedEvent& i) {
                const auto* p = e.try_get<CurrentPerformance>();

                if (p == nullptr)
                    return;

                p->performance->GetFsm().InsertEvent("InteractionFailed", i.interactionee);
            });
        });

    InitUtilitySystems(world);
}