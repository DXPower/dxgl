#pragma once

#include <modules/ai/Consideration.hpp>

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

struct Performer { };

struct CurrentPerformance {
    std::unique_ptr<Performance> performance{};
};

class PerformanceFactory {
public:
    virtual ~PerformanceFactory() = default;

    virtual std::unique_ptr<Performance> MakePerformance(flecs::entity_view potential_performance) = 0;
};

struct PerformanceFactoryStorage {
    std::unique_ptr<PerformanceFactory> factory{};
};

struct PotentialPerformanceScorer {
    std::shared_ptr<Consideration> consideration{};
};


// Anything that has this tag should also have:
// 1. PotentialPerformancePerformer relationship to which Performer can perform this action
// 2. PotentialPerformanceFactory relationship to which PerformanceFactory should create this action
//    (it's argument will be the entity with the PotentialPerformanceTag component)
// 3. (Optional): PotentialPerformanceTarget relationship(s) which the virtual PerformanceFactory can query for
// Virtual PerformanceFactories can also query for more data as needed.
struct PotentialPerformanceTag { };

struct PotentialPerformancePerformer { };
struct PotentialPerformanceTarget { };
struct PotentialPerformanceFactory { };

template<typename P>
class BasicPerformerPerformanceFactory final : public PerformanceFactory {
public:
    std::unique_ptr<Performance> MakePerformance(flecs::entity_view potential_performance) override {
        auto performer_e = potential_performance.target<PotentialPerformancePerformer>();
        return std::make_unique<P>(performer_e);
    };

};

}