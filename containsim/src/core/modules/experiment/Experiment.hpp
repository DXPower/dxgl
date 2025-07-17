#pragma once

#include <flecs.h>

// This module is simply for testing new features
namespace experiment {
    class Experiment {
    public:
        Experiment(flecs::world& world);
    };
}