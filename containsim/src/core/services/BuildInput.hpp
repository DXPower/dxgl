#pragma once

#include <services/commands/BuildInputCommands.hpp>
#include <services/commands/StateCommands.hpp>
#include <common/PimplHelper.hpp>

#include <common/ActionChain.hpp>
#include <services/commands/CommandChains.hpp>

namespace services {
    class BuildInput 
        : public ActionConsumer,
          public commands::CommandConsumer<commands::BuildInputCommand>
    {
    public:
        enum class StateId {
            IdleMode,
            BuildMode,
            PlaceTileMode,
            WorldTileSelectedMode,
            DeleteMode
        };

        commands::CommandProducer<commands::StateCommand> state_commands_out{};

    private:
        CREATE_PIMPL(m_pimpl);

    public:
        BuildInput();
        DECLARE_PIMPL_SMFS(BuildInput);

        void Consume(Action&& action) override;
        void Consume(commands::BuildInputCommandPtr&& command) override;

        void EnterBuildMode();
        void EnterDeleteMode();
        void SelectTileToPlace(TileType tile);
        void ExitMode();

        StateId GetState() const;
    };
}