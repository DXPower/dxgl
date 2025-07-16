#pragma once

enum class RoomInputStates {
    InactiveMode,
    IdleMode,
    DemarcationMode,
};

struct RoomInputStateChanged {
    RoomInputStates to;
};