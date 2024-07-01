#include <catch2/catch_test_macros.hpp>

#include <services/BuildMode.hpp>
#include <services/Logging.hpp>

#include <glfw/glfw3.h>

using namespace services;

struct StateCommandReceiver : ICommandReceiver<commands::StateCommand> {
    int build_mode_exit_count{};

    void PushCommand(commands::StateCommandPtr&& command) override {
        if (dynamic_cast<commands::StateExitMode*>(command.get()) != nullptr) {
            build_mode_exit_count++;
        }
    }
};

using State = BuildMode::StateType;

// TEST_CASE("BuildMode State Transitions", "[fsm][building]") {
//     StateCommandReceiver game_state_rec{};
//     BuildMode build_mode(game_state_rec);

//     SECTION("Default idle state") {
//         REQUIRE(build_mode.GetState() == State::Idle);
//     }

//     SECTION("Idle->BuildMode") {
//         build_mode.EnterBuildMode();
//         REQUIRE(build_mode.GetState() == State::BuildMode);

//         SECTION("BuildMode->Idle") {
//             build_mode.ExitMode();
//             REQUIRE(build_mode.GetState() == State::Idle);
//             REQUIRE(game_state_rec.build_mode_exit_count == 1);

//             SECTION("BuildMode->Idle->BuildMode") {
//                 build_mode.EnterBuildMode();
//                 REQUIRE(build_mode.GetState() == State::BuildMode);
//             }
//         }

//         SECTION("BuildMode->PlaceMode") {
//             build_mode.SelectTile(TileType::Tile);
//             REQUIRE(build_mode.GetState() == State::PlaceMode);
        
//             SECTION("PlaceMode->BuildMode") {
//                 build_mode.ExitMode();
//                 REQUIRE(build_mode.GetState() == State::BuildMode);

//                 SECTION("PlaceMode->BuildMode->Idle") {
//                     build_mode.ExitMode();
//                     REQUIRE(build_mode.GetState() == State::Idle);
//                     REQUIRE(game_state_rec.build_mode_exit_count == 1);
//                 }
//             }
//         }

//         SECTION("BuildMode->DeleteMode") {
//             build_mode.BeginDeleting();
//             REQUIRE(build_mode.GetState() == State::DeleteMode);

//             SECTION("DeleteMode->BuildMode") {
//                 build_mode.ExitMode();
//                 REQUIRE(build_mode.GetState() == State::BuildMode);
//             }
//         }
//     }
// }


TEST_CASE("BuildMode State Transitions", "[fsm][building][actions]") {
    StateCommandReceiver game_state_rec{};
    BuildMode build_mode(game_state_rec);

    build_mode.EnterBuildMode();
    
    constexpr glm::vec2 screen_pos = {250, 500};

    SECTION("Place tile") {
        build_mode.SelectTile(TileType::Tile);

        build_mode.PushAction(Action{
            .data = MouseClick{
                .dir = ButtonDir::Up,
                .pos = screen_pos,
                .button = GLFW_MOUSE_BUTTON_LEFT
            }
        });
    }

    SECTION("Delete tile") {
        build_mode.BeginDeleting();

        build_mode.PushAction(Action{
            .data = MouseClick{
                .dir = ButtonDir::Up,
                .pos = screen_pos,
                .button = GLFW_MOUSE_BUTTON_LEFT
            }
        });
    }
}