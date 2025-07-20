#include <modules/research/Researcher.hpp>
#include <modules/research/ResearchPoints.hpp>
#include <modules/ai/MovePerformance.hpp>
#include <modules/core/Transform.hpp>
#include <modules/core/Interaction.hpp>
#include <modules/core/Science.hpp>

using namespace research;

ResearchPerformance::ResearchPerformance(flecs::entity performer) 
    : Performance(performer, "ResearchPerformance") {

    SetupDefaultStates();
    StateResearch(m_fsm, "Research");
    ai::StateMovePerformance(m_fsm, "MoveToResearchPoint", {
        .self = this
    });

    m_fsm.AddTransition(ai::DefaultPerformerStates::Begin, ai::MovePerformanceEvents::BeginMoving::Id, "MoveToResearchPoint");
    m_fsm.AddTransition("MoveToResearchPoint", ai::DefaultPerformerEvents::SubstepSuccess, "Research");
    m_fsm.AddTransition("Research", ai::DefaultPerformerEvents::SubstepSuccess, "Complete");
    m_fsm.SetCurrentState(ai::DefaultPerformerStates::Begin);
}

auto ResearchPerformance::StateBegin(FSM& fsm, StateId_t) -> State {
    Event ev{};

    while (true) {
        auto reset_token = co_await fsm.EmitAndReceiveResettable(ev);

        if (reset_token.ShouldReset()) {
            m_performer.remove<AwaitingResearchPoint>();
        } else {
            m_performer.add<AwaitingResearchPoint>();
        }
        
        if (ev == ai::DefaultPerformerEvents::Tick) {
            if (flecs::entity point = m_performer.target<AssignedResearchPoint>()) {
                const auto& transform = point.get<core::Transform>();

                ev.Store(ai::MovePerformanceEvents::BeginMoving::Id, ai::MovePerformanceEvents::BeginMoving{
                    .destination = transform.position
                });
                continue;
            }
        }

        ev.Clear();
    }
}

auto ResearchPerformance::StateResearch(FSM& fsm, StateId_t) -> State {
    Event ev{};

    while (true) {
        flecs::entity point_e;
        co_await fsm.ReceiveEvent(ev);

        // First time we enter here, start interacting
        point_e = m_performer.target<AssignedResearchPoint>();
        m_performer.add<core::InteractingWith>(point_e);
        ev.Clear();

        // Wait until we succeed or are interrupted
        while (!co_await fsm.EmitAndReceiveResettable(ev)) {
            if (ev == ai::DefaultPerformerEvents::InteractionComplete) {
                // Success
                auto point = point_e.get<ResearchPoint>();

                point_e.set(core::ScienceGain{point.science_per_success});
                ev = ai::DefaultPerformerEvents::SubstepSuccess;
                continue;
            } else if (ev == ai::DefaultPerformerEvents::InteractionFailed) {
                // Try again
                break;
            }

            ev.Clear();
        }
    }
}
