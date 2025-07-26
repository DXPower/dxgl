#pragma once

#include <flecs.h>
#include <modules/ai/Perform.hpp>

namespace ai {

struct UtilityPicker { };
struct UtilityScore {
    float value{};
};

void InitUtilitySystems(flecs::world& world);

}