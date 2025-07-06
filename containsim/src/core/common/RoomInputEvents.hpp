#pragma once

enum class RoomInputStates {
    IdleMode,
    DemarcationMode,
};

struct RoomInputStateChanged {
    RoomInputStates to;
};