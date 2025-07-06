#pragma once

enum class InputStates {
    IdleMode,
    PauseMenu,
    BuildActive,
    RoomActive,
};

struct InputStateChanged {
    InputStates to{};
};