#pragma once

namespace services {

enum class InputStates {
    IdleMode,
    PauseMenu,
    BuildActive
};

struct InputStateChanged {
    InputStates to{};
};

}