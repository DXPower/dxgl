#pragma once

enum class BuildInputStates {
    InactiveMode,
    IdleMode,
    PlaceTileMode,
    PlaceTileDragMode,
    WorldTileSelectedMode,
    DeleteMode
};

struct BuildInputStateChanged {
    BuildInputStates to;
};