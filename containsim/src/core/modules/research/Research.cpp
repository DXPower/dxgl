#include <modules/research/Research.hpp>
#include <modules/core/Core.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/ai/Ai.hpp>

#include <common/Logging.hpp>

using namespace research;

Research::Research(flecs::world& world) {
    auto logger = logging::CreateSharedLogger("Research");

    world.import<core::Core>();
    world.import<pathing::Pathing>();
    world.import<physics::Physics>();
    world.import<ai::Ai>();

    world.component<Researcher>();
    world.component<ResearchPoint>();
    world.component<ResearchPointReserved>();
    world.component<AwaitingResearchPoint>();
    world.component<AssignedResearchPoint>().add(flecs::Relationship);

    auto find_free_research_points = world.query_builder()
        .with<ResearchPoint>()
        .without<ResearchPointReserved>()
        .without<core::Cooldown>()
        .build();

    world.system("AssignResearchers")
        .immediate()
        .with<AwaitingResearchPoint>()
        .without<AssignedResearchPoint>(flecs::Wildcard)
        .each([find_free_research_points, logger](flecs::entity researcher) {
            auto research_point = find_free_research_points.first();

            if (!research_point)
                return;

            research_point.add<ResearchPointReserved>();
            researcher.add<AssignedResearchPoint>(research_point);
            logger->debug("Assigning research point with id {}", research_point.id());
        });

    // Setup the research performance
    auto research_performance_factory = world.entity("ResearchPerformanceFactory")
        .set(ai::PerformanceFactoryStorage{
            .factory = std::make_unique<ai::BasicPerformerPerformanceFactory<ResearchPerformance>>()
        });

    auto research_performance_scorer = world.entity("ResearchPerformanceScorer")
        .set(ai::PotentialPerformanceScorer{
            .consideration = std::make_shared<ai::ConstantConsideration>("ResearchConstant", 0.75f)
        });

    world.observer()
        .with<Researcher>()
        .event(flecs::OnAdd)
        .yield_existing()
        .each([=](flecs::entity e) {
            using namespace ai;
            e.world().entity()
                .add<PotentialPerformanceTag>()
                .add<PotentialPerformanceFactory>(research_performance_factory)
                .add<PotentialPerformanceScorer>(research_performance_scorer)
                .add<PotentialPerformancePerformer>(e);
        });
}