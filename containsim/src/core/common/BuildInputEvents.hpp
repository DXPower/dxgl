#pragma once

enum class BuildInputStates {
    InactiveMode,
    IdleMode,
    PlaceTileMode,
    PlaceTileDragMode,
    WorldTileSelectedMode,
    DeleteMode,
    DeleteDragMode,
};

struct BuildInputStateChanged {
    BuildInputStates to;
};