#include <services/commands/RoomInputCommands.hpp>
#include <modules/input/RoomInput.hpp>

using namespace services;
using namespace services::commands;
using namespace input;

void SelectRoomType::Execute(RoomInput& fsm) const {
    fsm.SelectRoomType(type);
}

void SelectRoomClear::Execute(RoomInput& fsm) const {
    fsm.SelectRoomClear();
}

void ExitRoomMode::Execute(RoomInput& fsm) const {
    fsm.ExitMode();
}

void ResetRoomInput::Execute(RoomInput& fsm) const {
    fsm.GetFsm().SetCurrentState(RoomInput::StateId::IdleMode);
}
