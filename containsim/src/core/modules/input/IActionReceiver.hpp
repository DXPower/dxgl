#pragma once

#include <common/Action.hpp>

namespace input {
    struct IActionReceiver {
        virtual ~IActionReceiver() = default;

        virtual void PushAction(Action&& action) = 0;
    };
}