#pragma once

#include <flecs.h>

namespace prefabs {
    struct Researcher { };
    struct ResearchDesk { };

    class Prefabs {
    public:
        Prefabs(flecs::world& world);
    };
}