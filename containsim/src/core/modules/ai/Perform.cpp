#include <modules/ai/Perform.hpp>

using namespace ai;

Performance::Performance(flecs::entity performer, std::string name)
    : m_performer(performer)
    , m_logger(logging::CreateLogger(std::move(name))) { }

void Performance::SetupDefaultStates() {
    StateBegin(m_fsm, DefaultPerformerStates::Begin);
    StateComplete(m_fsm, DefaultPerformerStates::Complete);
}

auto Performance::StateBegin(FSM& fsm, StateId_t) -> State {
    while (true) {
        co_await fsm.IgnoreEvent();
    }
}

auto Performance::StateComplete(FSM& fsm, StateId_t) -> State {
    while (true) {
        co_await fsm.IgnoreEvent();
    }
}

