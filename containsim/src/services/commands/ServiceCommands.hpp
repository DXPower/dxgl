#pragma once

#include <variant>
#include <memory>

namespace services {
    class BuildManager;

    namespace commands {
        struct BuildCommand {
            virtual ~BuildCommand() = default;

            virtual void Execute(BuildManager& manager) = 0;
        };

        
        using ServiceCommand = std::variant<std::unique_ptr<BuildCommand>>;
    }
}