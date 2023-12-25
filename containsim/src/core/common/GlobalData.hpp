#pragma once

#include "common/DrawQueues.hpp"
#include <common/GlobalConfig.hpp>
#include <common/GlobalState.hpp>

struct GlobalData {
    GlobalConfig* config{};
    GlobalState* state{};
    DrawQueues* draw_queues{};
};