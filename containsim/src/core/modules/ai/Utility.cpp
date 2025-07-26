#include <modules/ai/Utility.hpp>
#include <modules/ai/Consideration.hpp>

#include <algorithm>

using namespace ai;


void ai::InitUtilitySystems(flecs::world& world) {
    world.component<PerformanceFactoryStorage>();
    world.component<PotentialPerformanceTag>();
    world.component<PotentialPerformanceScorer>();
    world.component<PotentialPerformanceFactory>().add(flecs::Relationship);
    world.component<PotentialPerformancePerformer>().add(flecs::Relationship);
    world.component<PotentialPerformanceTarget>().add(flecs::Relationship);

    world.component<UtilityPicker>();
    world.component<UtilityScore>();

    world.system("ScorePotentialPerformances")
        .with<PotentialPerformanceTag>()
        .with<PotentialPerformanceScorer>().second(flecs::Wildcard)
        .with<PotentialPerformancePerformer>().second(flecs::Wildcard)
        .with<PotentialPerformanceTarget>().second(flecs::Wildcard).optional()
        .write<UtilityScore>()
        .kind(flecs::PreUpdate)
        .each([](flecs::iter& it, size_t idx) {
            flecs::entity potential_performance = it.entity(idx);

            ConsiderationContext context{};
            context.potential_performance = potential_performance;
            context.performer = it.pair(2).second();

            if (it.is_set(3)) {
                context.target = it.pair(3).second();
            }

            const auto scorer_e = it.pair(1).second();
            const auto& scorer = scorer_e.get<PotentialPerformanceScorer>();
            const float score = scorer.consideration->GetValue(context);

            potential_performance.set(UtilityScore{.value = score});
        });

    world.system<Performer>("SetCurrentAction")
        .read<UtilityScore>()
        .read<PerformanceFactoryStorage>()
        .kind(flecs::PreUpdate)
        .each([](flecs::entity e, Performer& performer) {
            if (performer.performance != nullptr) {
                auto cur_state = performer.performance->GetFsm().GetCurrentState();
                // If there is no state set, we're considering it not started.
                // Should probably change this in the future to detect error states
                bool still_running = !cur_state.has_value() || cur_state->Id() != DefaultPerformerStates::Complete;
                if (still_running) {
                    // Don't select a new action until it's done
                    // TODO: Interruptions
                    return;
                } else {
                    performer.performance.reset();
                }
            }

            struct ScoredPerformance {
                flecs::entity performance{};
                float score{};
            };
            
            std::vector<ScoredPerformance> scored_perfs{};

            e.world().query_builder<const UtilityScore>()
                .with<PotentialPerformancePerformer>().second(e)
                .build()
                .each([&](flecs::iter& it, size_t idx, const UtilityScore& score) {
                    scored_perfs.push_back({
                        .performance = it.entity(idx),
                        .score = score.value
                    });
                });

            if (scored_perfs.empty())
                return;

            // TODO: Choose random from top few scores
            auto highest = std::ranges::max_element(scored_perfs, {}, &ScoredPerformance::score);
            
            // Create the performance and assign it to the performer
            auto factory_e = highest->performance.target<PotentialPerformanceFactory>();
            const auto& factory = factory_e.get<PerformanceFactoryStorage>().factory;

            auto new_performance = factory->MakePerformance(highest->performance);
            performer.performance = std::move(new_performance);
            performer.performance->GetFsm().InsertEvent(ai::DefaultPerformerEvents::Begin);
        });
}