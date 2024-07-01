#pragma once

#include <services/commands/BuildModeCommands.hpp>
#include <services/commands/StateCommands.hpp>
#include <common/PimplHelper.hpp>

#include <common/ActionChain.hpp>
#include <services/commands/CommandChains.hpp>

namespace services {
    class BuildMode 
        : public ActionConsumer,
          public commands::CommandConsumer<commands::BuildModeCommand>
    {
    public:
        enum class StateType {
            Idle,
            BuildMode,
            PlaceMode,
            DeleteMode
        };

        commands::CommandProducer<commands::StateCommand> state_commands_out{};

    private:
        CREATE_PIMPL(m_pimpl);

    public:
        BuildMode();

        DECLARE_PIMPL_SMFS(BuildMode);

        void Consume(Action&& action) override;
        void Consume(commands::BuildModeCommandPtr&& command) override;

        void EnterBuildMode();
        void SelectTile(TileType tile);

        void ExitMode();

        void BeginDeleting();
        void EndDeleting();

        StateType GetState() const;
    };
}