#include <modules/ai/CommonConsiderations.hpp>

using namespace ai;

float DistanceConsideration::GetValue(const ConsiderationContext& context) const {
    const auto& t1 = context.performer.get<core::Transform>();
    const auto& t2 = context.target.get<core::Transform>();
    return glm::distance(t1.position, t2.position);
}