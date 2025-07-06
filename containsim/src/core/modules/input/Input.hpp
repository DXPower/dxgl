#pragma once

#include <modules/input/BuildInput.hpp>
#include <modules/input/InputState.hpp>
#include <modules/input/RoomInput.hpp>
#include <modules/input/BasicMouseTester.hpp>
#include <modules/input/UiActionReceiver.hpp>
#include <modules/input/ActionRouter.hpp>
#include <modules/input/InputHandler.hpp>

#include <flecs.h>

namespace input {

class Input {
public:
    Input(flecs::world& world);
};

}