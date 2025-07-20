#pragma once

#include <modules/research/ResearchPoints.hpp>
#include <modules/research/Researcher.hpp>

#include <flecs.h>

namespace research {

class Research {
public:
    Research(flecs::world& world);
};

}