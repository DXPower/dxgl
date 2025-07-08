#include <modules/experiment/Experiment.hpp>

#include <modules/core/Core.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/rendering/Rendering.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/input/Input.hpp>

#include <components/Actor.hpp>
#include <components/Transform.hpp>
#include <components/Mobility.hpp>

#include <common/Logging.hpp>
#include <common/ActionChain.hpp>

#include <GLFW/glfw3.h>

using namespace experiment;

namespace {
// bool toggle_debugger = false;
// std::optional<glm::vec2> target_pos{};

// if (toggle_debugger) {
//     toggle_debugger = false;
//     Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
// }

struct GlobalActions : ActionConsumer {
    flecs::entity target{};
    const rendering::Camera* cam{};

    GlobalActions(flecs::entity target, const rendering::Camera& cam) : target(target), cam(&cam) { }

    void Consume(Action&& action) override {
        // const KeyPress* press = std::get_if<KeyPress>(&action.data);
        // if (press != nullptr && press->IsDownKey(GLFW_KEY_F8)) {
        //     toggle_debugger = true;
        // }

        const MouseClick* click = std::get_if<MouseClick>(&action.data);
        if (click != nullptr && click->button == 0 && click->dir == ButtonDir::Up) {
            // target_pos = click->pos;
            const auto cam_view = cam->GetViewMatrix();
            const auto world_pos = glm::inverse(cam_view) * glm::vec4(click->pos, 1, 1);

            target.set(pathing::DestinationIntent{.position = world_pos});
            target.add<pathing::StaleDestination>();
        }
    }
};
}

Experiment::Experiment(flecs::world& world) {
    world.import<core::Core>();
    world.import<physics::Physics>();
    world.import<rendering::Rendering>();
    world.import<pathing::Pathing>();
    world.import<input::Input>();

    auto logger = logging::CreateSharedLogger("Experiment");

    const auto& tile_grid = world.get<core::TileGrid>();
    auto& build_manager = world.get_mut<core::BuildManager>();

    const auto tile_sel = TileSelection{
        .start = {0, 0},
        .end = tile_grid.GetGridSize() - glm::ivec2{1, 1}
    };

    for (auto tile_coord : tile_sel.Iterate()) {
        build_manager.PlaceTile(tile_coord, TileType::Grass);
    }

    struct PlayerTest { };
    world.component<PlayerTest>();

    auto gardner_sheet_e = world.entity("GardnerSheet")
        .set<dxgl::Texture>(dxgl::LoadTextureFromFile("res/img/gardner.png"));

    const auto& gardner_sheet = gardner_sheet_e.get<dxgl::Texture>();

    // Populate world actors
    auto test_actor = world.entity().set_name("TestActor")
        .set<components::Actor>(components::Actor{.id = 1})
        .add<PlayerTest>()
        .set<pathing::PathMover>(pathing::PathMover{})
        .set(components::Transform{
            .position = {400, 400},
            .size = {95, 95},
        })
        .set(rendering::Sprite{
            .spritesheet = gardner_sheet,
            .cutout = {
                .position = {0, 0},
                .size = {64, 64}
            }
        })
        .set(rendering::RenderData{
            .layer = RenderLayer::Objects
        })
        .add<rendering::SpriteRenderer>()
        .set<components::Mobility>(components::Mobility{.speed = 350.f})
        .set(physics::Collider{
            .is_listening = true
        })
        .set(physics::SquareCollider{
            .relative_size = {.9f, .95f}
        });
    
    for (int i = 0; i < 2; i++) {
        auto other_actor = world.entity();
        other_actor.set<components::Actor>(components::Actor{.id = 2 + (unsigned int)i});
        other_actor.set(components::Transform{
            .position = {100 + (105 * i), 100},
            .size = {200, 200},
        });
        other_actor.set(rendering::Sprite{
            .spritesheet = gardner_sheet,
            .cutout = {
                .position = {0, 64},
                .size = {64, 64}
            }
        });
        other_actor.set(rendering::RenderData{
            .layer = RenderLayer::Objects
        });
        other_actor.add<rendering::SpriteRenderer>();
        other_actor.set(physics::Collider{
            .is_trigger = true,
            .is_listening = false
        });
        other_actor.set(physics::SquareCollider{
            .relative_size = {.5f, .9f}
        });
    }

    world.system<physics::CollisionBegan, PlayerTest>("PlayerCollisionBegan")
        .term_at(0).second(flecs::Wildcard)
        .kind(flecs::OnUpdate)
        .each([&, logger](flecs::iter& it, size_t, physics::CollisionBegan, PlayerTest) {
            logger->info("Player collided with something!");
            auto other = it.pair(0).second();

            if (other.has<components::Actor>()) {
                const auto& other_actor = other.get<components::Actor>();
                logger->info("Player collided with actor {}", other_actor.id);
            }
        });

    world.system<physics::CollisionEnded, PlayerTest>("PlayerCollisionEnded")
        .term_at(0).second(flecs::Wildcard)
        .kind(flecs::OnUpdate)
        .each([&, logger](flecs::iter& it, size_t, physics::CollisionEnded, PlayerTest) {
            logger->info("Player stopped colliding with something!");
            auto other = it.pair(0).second();

            if (other.has<components::Actor>()) {
                const auto& other_actor = other.get<components::Actor>();
                logger->info("Player stopped colliding with actor {}", other_actor.id);
            }
        });

    const auto& camera = world.get<rendering::Camera>();

    world.component<GlobalActions>().add(flecs::Sparse);
    world.emplace<GlobalActions>(test_actor, camera);
    auto& global_actions = world.get_mut<GlobalActions>();

    auto& build_input = world.get_mut<input::BuildInput>();
    auto& room_input = world.get_mut<input::RoomInput>();
    auto& input_state = world.get_mut<input::InputState>();
    
    chain::Connect(input_state.idle_actions, global_actions);
    chain::Connect(build_input.uncaptured_actions, global_actions);
    chain::Connect(room_input.uncaptured_actions, global_actions);
}