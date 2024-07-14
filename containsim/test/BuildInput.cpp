#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <services/BuildInput.hpp>
#include <services/Logging.hpp>

#include <glfw/glfw3.h>

using namespace services;

namespace {
    struct StateCommandReceiver : commands::CommandConsumer<commands::StateCommand> {
        int build_input_exit_count{};

        void Consume(commands::StateCommandPtr&& command) override {
            if (dynamic_cast<commands::StateExitMode*>(command.get()) != nullptr) {
                build_input_exit_count++;
            }
        }
    };

    using State = BuildInput::StateId;
}

TEST_CASE("BuildMode State Transitions", "[fsm][building]") {
    StateCommandReceiver game_state_rec{};
    BuildInput build_input{};

    chain::Connect(build_input.state_commands_out, game_state_rec);

    using Strat = std::function<void(BuildInput&)>;

    Strat exit_strat = GENERATE(
        Strat{[](BuildInput& b) { b.ExitMode(); }},
        Strat{[](BuildInput& b) { b.Consume(Action{KeyPress{.dir = ButtonDir::Down, .key = GLFW_KEY_ESCAPE}}); }}
    );

    SECTION("Default idle state") {
        REQUIRE(build_input.GetState() == State::IdleMode);
    }

    SECTION("Idlemode->BuildMode") {
        build_input.EnterBuildMode();
        REQUIRE(build_input.GetState() == State::BuildMode);

        SECTION("BuildMode->Idlemode") {
            exit_strat(build_input);
            REQUIRE(build_input.GetState() == State::IdleMode);
            REQUIRE(game_state_rec.build_input_exit_count == 1);

            SECTION("BuildMode->Idlemode->BuildMode") {
                build_input.EnterBuildMode();
                REQUIRE(build_input.GetState() == State::BuildMode);
            }
        }

        SECTION("BuildMode->PlaceTileMode") {
            build_input.SelectTileToPlace(TileType::Tile);
            REQUIRE(build_input.GetState() == State::PlaceTileMode);
        
            SECTION("PlaceTileMode->BuildMode") {
                exit_strat(build_input);
                REQUIRE(build_input.GetState() == State::BuildMode);

                SECTION("PlaceTileMode->BuildMode->Idlemode") {
                    exit_strat(build_input);
                    REQUIRE(build_input.GetState() == State::IdleMode);
                    REQUIRE(game_state_rec.build_input_exit_count == 1);
                }
            }
        }

        SECTION("DeleteMode") {
            Strat enter_delete_strat = GENERATE(
                Strat{[](BuildInput& b) { b.EnterDeleteMode(); }},
                Strat{[](BuildInput& b) { 
                    b.EnterBuildMode();
                    b.Consume(Action{KeyPress{.dir = ButtonDir::Down, .key = GLFW_KEY_R}});
                }},
                Strat{[](BuildInput& b) { 
                    b.SelectTileToPlace(TileType::Dirt);
                    b.Consume(Action{KeyPress{.dir = ButtonDir::Down, .key = GLFW_KEY_R}});
                }}
            );

            enter_delete_strat(build_input);
            REQUIRE(build_input.GetState() == State::DeleteMode);

            SECTION("DeleteMode->BuildMode") {
                exit_strat(build_input);
                REQUIRE(build_input.GetState() == State::BuildMode);
            }

            SECTION("DeleteMode->PlaceTileMode") {
                build_input.SelectTileToPlace(TileType::Dirt);
                REQUIRE(build_input.GetState() == State::PlaceTileMode);
            }
        }
    }
}
