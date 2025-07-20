#pragma once

#include <common/Ticks.hpp>
#include <cstdint>
namespace research {
    struct ResearchPoint {
        int64_t science_per_success{};
    };

    struct ResearchPointReserved { };
}