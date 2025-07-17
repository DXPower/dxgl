#pragma once

enum class RoomInputStates {
    InactiveMode,
    IdleMode,
    DemarcationMode,
    DemarcationDragMode
};

struct RoomInputStateChanged {
    RoomInputStates to;
};