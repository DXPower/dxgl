#pragma once

#include <common/Logging.hpp>
#include <dxfsm/dxfsm.hpp>
#include <flecs.h>

namespace ai {

namespace DefaultPerformerStates {
    inline const std::string Begin = "Begin";
    inline const std::string Complete = "Complete";
}

namespace DefaultPerformerEvents {
    inline const std::string Begin = "Begin";
    inline const std::string SelfInterrupt = "SelfInterrupt";
    inline const std::string ForceInterrupt = "ForceInterrupt";
    inline const std::string Serialize = "Serialize";
    inline const std::string Tick = "Tick";
    inline const std::string InteractionComplete = "InteractionComplete";
    inline const std::string InteractionFailed = "InteractionFailed";
    inline const std::string SubstepSuccess = "SubstepSuccess";
    inline const std::string SubstepFailure = "SubstepFailure";
}

class Performance {
public:
    using StateId_t = std::string;
    using EventId_t = std::string;
    using State = dxfsm::State<StateId_t>;
    using Event = dxfsm::Event<EventId_t>;
    using FSM = dxfsm::FSM<StateId_t, EventId_t>;

protected:
    FSM m_fsm{};
    flecs::entity m_performer{};
    logging::Logger m_logger;

public:
    Performance(flecs::entity performer, std::string name);
    virtual ~Performance() = default;

    virtual State StateBegin(FSM& fsm, StateId_t);
    virtual State StateComplete(FSM& fsm, StateId_t);

    flecs::entity GetPerformer() const {
        return m_performer;
    }

    FSM& GetFsm() { return m_fsm; }
    const FSM& GetFsm() const { return m_fsm; }

protected:
    void SetupDefaultStates();
};

struct Performer {
    std::unique_ptr<Performance> performance{};
};

}