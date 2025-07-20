#include <modules/research/Research.hpp>
#include <modules/core/Core.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/physics/Physics.hpp>
#include <common/Logging.hpp>

using namespace research;

Research::Research(flecs::world& world) {
    auto logger = logging::CreateSharedLogger("Research");

    world.import<core::Core>();
    world.import<pathing::Pathing>();
    world.import<physics::Physics>();

    world.component<Researcher>();
    world.component<ResearchPoint>();
    world.component<ResearchPointReserved>();

    auto find_free_research_points = world.query_builder()
        .with<ResearchPoint>()
        .without<ResearchPointReserved>()
        .cache_kind(flecs::QueryCacheAll)
        .build();

    world.system("AssignResearchers")
        .immediate()
        .with<AwaitingResearchPoint>()
        .each([find_free_research_points, logger](flecs::entity researcher) {
            auto research_point = find_free_research_points.first();

            if (!research_point)
                return;

            research_point.add<ResearchPointReserved>();
            researcher.add<AssignedResearchPoint>(research_point);
            logger->debug("Assigning research point with id {}", research_point.id());
        });

    // world.system("BeginResearching")
    //     .with<Researcher>()
    //     .with<IntentToResearch>()
    //     .with<physics::CollisionBegan>(flecs::Wildcard)
    //     .each([logger](flecs::iter& it, size_t idx) {
    //         flecs::entity collision = it.pair(2).second();
    //         flecs::entity self = it.entity(idx);

    //         if (collision.has<ResearchPoint>()) {
    //             self.remove<pathing::DestinationIntent>();
    //             self.remove<pathing::Path>();
    //         }
    //     });

}