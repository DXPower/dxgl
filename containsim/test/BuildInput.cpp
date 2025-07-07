#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <services/BuildInput.hpp>
#include <common/Logging.hpp>

#include <glfw/glfw3.h>

using namespace services;

namespace {
    using State = BuildInput::StateId;
}

TEST_CASE("IdleMode State Transitions", "[fsm][building]") {
    BuildInput build_input{};

    using Strat = std::function<void(BuildInput&)>;

    Strat exit_strat = GENERATE(
        Strat{[](BuildInput& b) { b.ExitMode(); }},
        Strat{[](BuildInput& b) { b.Consume(Action{KeyPress{.dir = ButtonDir::Down, .key = GLFW_KEY_ESCAPE}}); }}
    );

    SECTION("Default state") {
        REQUIRE(build_input.GetState() == State::IdleMode);
    }

    SECTION("IdleMode->PlaceTileMode") {
        build_input.SelectTileToPlace(TileType::Tile);
        REQUIRE(build_input.GetState() == State::PlaceTileMode);
    
        SECTION("PlaceTileMode->IdleMode") {
            exit_strat(build_input);
            REQUIRE(build_input.GetState() == State::IdleMode);
        }
    }

    SECTION("DeleteMode") {
        Strat enter_delete_strat = GENERATE(
            Strat{[](BuildInput& b) { b.EnterDeleteMode(); }},
            Strat{[](BuildInput& b) { 
                b.Consume(Action{KeyPress{.dir = ButtonDir::Down, .key = GLFW_KEY_R}});
            }},
            Strat{[](BuildInput& b) { 
                b.SelectTileToPlace(TileType::Dirt);
                b.Consume(Action{KeyPress{.dir = ButtonDir::Down, .key = GLFW_KEY_R}});
            }}
        );

        enter_delete_strat(build_input);
        REQUIRE(build_input.GetState() == State::DeleteMode);

        SECTION("DeleteMode->IdleMode") {
            exit_strat(build_input);
            REQUIRE(build_input.GetState() == State::IdleMode);
        }

        SECTION("DeleteMode->PlaceTileMode") {
            build_input.SelectTileToPlace(TileType::Dirt);
            REQUIRE(build_input.GetState() == State::PlaceTileMode);
        }
    }
}
