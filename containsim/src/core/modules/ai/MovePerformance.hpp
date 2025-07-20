#pragma once

#include <modules/ai/Perform.hpp>
#include <glm/vec2.hpp>

namespace ai {
    struct MovePerformanceConfig {
        Performance* self{};
    };

    namespace MovePerformanceEvents {
        struct BeginMoving {
            inline static const std::string Id = "BeginMoving";
            glm::vec2 destination{};
        };
    };

    Performance::State StateMovePerformance(Performance::FSM& fsm, Performance::StateId_t, MovePerformanceConfig c);
}