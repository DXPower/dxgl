#pragma once

namespace services {
    enum class RoomInputStates {
        IdleMode,
        DemarcationMode,
    };

    struct RoomInputStateChanged {
        RoomInputStates to;
    };
}