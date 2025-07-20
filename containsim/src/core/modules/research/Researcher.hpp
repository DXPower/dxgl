#pragma once

#include <modules/ai/Perform.hpp>

namespace research {

struct Researcher {
    float research_efficiency{};
};

struct AssignedResearchPoint { };
struct AwaitingResearchPoint { };

class ResearchPerformance final : public ai::Performance {
public:
    // struct ResearchPointAssigned {
    //     inline static const std::string Id = "ResearchPointAssigned";
    //     flecs::entity research_point{};
    // };

    ResearchPerformance(flecs::entity performer);

    State StateBegin(FSM& fsm, StateId_t) override;
    State StateResearch(FSM& fsm, StateId_t);
};

}