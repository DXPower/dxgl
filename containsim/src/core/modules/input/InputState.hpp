#pragma once

#include <common/ActionChain.hpp>
#include <common/InputStateEvents.hpp>
#include <common/Logging.hpp>
#include <modules/application/EventManager.hpp>
#include <common/EventCommandable.hpp>
#include <common/MeceFsm.hpp>
#include <flecs.h>

namespace input {
void SetupInputState(flecs::world& world);
}