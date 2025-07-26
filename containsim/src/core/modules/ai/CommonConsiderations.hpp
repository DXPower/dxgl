#pragma once

#include <modules/ai/Consideration.hpp>
#include <modules/core/Transform.hpp>
#include <glm/glm.hpp>

namespace ai {
    class DistanceConsideration final : public Consideration {
    public:
        DistanceConsideration(std::string name) : Consideration(std::move(name)) { }

        float GetValue(const ConsiderationContext& context) const override;
    };
}