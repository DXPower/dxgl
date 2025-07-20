#include <modules/ai/MovePerformance.hpp>
#include <modules/pathing/PathingComponents.hpp>
using namespace ai;

Performance::State ai::StateMovePerformance(Performance::FSM& fsm, Performance::StateId_t, MovePerformanceConfig c) {
    Performance::Event ev{};
    flecs::entity performer = c.self->GetPerformer();

    while (true) {
        auto reset_token = co_await fsm.EmitAndReceiveResettable(ev);

        if (reset_token.ShouldReset()) {
            // When we leave the movement state, remove the destination intent
            performer.remove<pathing::DestinationIntent>();
        } else if (ev == MovePerformanceEvents::BeginMoving::Id) {
            auto data = ev.Get<MovePerformanceEvents::BeginMoving>();

            performer.add<pathing::StaleDestination>();
            performer.set(pathing::DestinationIntent{
                .position = data.destination
            });
        } else if (ev == DefaultPerformerEvents::Tick) {
            // We have reached the destination, return success!
            if (!performer.has<pathing::Path>()) {
                ev = DefaultPerformerEvents::SubstepSuccess;
                continue;
            }
        }

        ev.Clear();
    }
}

