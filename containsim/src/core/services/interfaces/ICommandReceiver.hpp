#pragma once

#include <services/commands/Command.hpp>

namespace services {

    template<typename T>
    struct ICommandReceiver {
        virtual ~ICommandReceiver() = default;

        virtual void PushCommand(commands::CommandPtr<T>&& command) = 0;
    };
}