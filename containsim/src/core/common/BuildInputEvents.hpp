#pragma once

enum class BuildInputStates {
    InactiveMode,
    IdleMode,
    PlaceTileMode,
    WorldTileSelectedMode,
    DeleteMode
};

struct BuildInputStateChanged {
    BuildInputStates to;
};