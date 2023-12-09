#pragma once

#include <common/Action.hpp>

namespace services {
    struct IActionReceiver {
        virtual ~IActionReceiver() = default;

        virtual void PushAction(Action&& action) = 0;
    };
}