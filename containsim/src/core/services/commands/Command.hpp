#pragma once

#include <memory>

namespace services::commands {
    template<typename Executee>
    struct Command {
        virtual ~Command() = default;

        virtual void Execute(Executee& executee) const = 0;
    };

    template<typename CommandType>
    using CommandPtr = std::unique_ptr<CommandType>;

    template<typename CommandType, typename... Args>
    CommandPtr<CommandType> MakeCommandPtr(Args&&... args) {
        return std::make_unique<CommandType>(std::forward<Args>(args)...);
    }
}