#pragma once

namespace services {

enum class InputStates {
    IdleMode,
    PauseMenu,
    BuildActive,
    RoomActive,
};

struct InputStateChanged {
    InputStates to{};
};

}