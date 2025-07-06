#pragma once

enum class BuildInputStates {
    IdleMode,
    PlaceTileMode,
    WorldTileSelectedMode,
    DeleteMode
};

struct BuildInputStateChanged {
    BuildInputStates to;
};