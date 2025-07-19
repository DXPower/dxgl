#include <modules/experiment/Experiment.hpp>

#include <modules/core/Core.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/rendering/Rendering.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/input/Input.hpp>

#include <modules/core/Actor.hpp>
#include <modules/core/Transform.hpp>
#include <modules/core/Mobility.hpp>

#include <common/Logging.hpp>
#include <common/ActionChain.hpp>
#include <common/Ticks.hpp>

#include <GLFW/glfw3.h>

using namespace experiment;
using namespace core;
using namespace rendering;

namespace {
// bool toggle_debugger = false;
// std::optional<glm::vec2> target_pos{};

// if (toggle_debugger) {
//     toggle_debugger = false;
//     Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
// }

struct GlobalActions : ActionConsumer {
    flecs::entity target{};
    flecs::entity tick_source{};
    float ticks_per_second{4.f};

    MeceFsm* input_state{};
    const rendering::Camera* cam{};

    GlobalActions(flecs::entity target, flecs::entity tick_source, const rendering::Camera& cam)
        : target(target), tick_source(tick_source), cam(&cam) {
        input_state = &target.world().lookup("input::Input::InputState")
            .get_mut<MeceFsm>();
    }

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

        const KeyPress* key = std::get_if<KeyPress>(&action.data);
        if (key != nullptr && key->dir == ButtonDir::Down) {
            switch (key->key) {
                case GLFW_KEY_B:
                    input_state->GetSubFsm("BuildInput")->MakeActive();
                    break;
                case GLFW_KEY_R:
                    input_state->GetSubFsm("RoomInput")->MakeActive();
                    break;
                case GLFW_KEY_P:
                    if (tick_source.has<PauseTicks>()) {
                        tick_source.remove<PauseTicks>();
                    } else {
                        tick_source.add<PauseTicks>();
                    }
                    break;
                case GLFW_KEY_COMMA:
                    ticks_per_second /= 2;
                    tick_source.world().set<TickRate>(TickRate{
                        .period = ch::seconds(1) / ticks_per_second
                    });
                    break;
                case GLFW_KEY_PERIOD:
                    ticks_per_second *= 2;
                    tick_source.world().set<TickRate>(TickRate{
                        .period = ch::seconds(1) / ticks_per_second
                    });
                    break;
                default:
                    break;
            }
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
        .set<core::Actor>(core::Actor{.id = 1})
        .add<PlayerTest>()
        .set<pathing::PathMover>(pathing::PathMover{})
        .set(core::Transform{
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
        .set<core::Mobility>(core::Mobility{.speed = 87.5f})
        .set(physics::Collider{
            .is_listening = true
        })
        .set(physics::SquareCollider{
            .relative_size = {.9f, .95f}
        });
    
    for (int i = 0; i < 2; i++) {
        auto other_actor = world.entity();
        other_actor.set<core::Actor>(core::Actor{.id = 2 + (unsigned int)i});
        other_actor.set(core::Transform{
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

            if (other.has<core::Actor>()) {
                const auto& other_actor = other.get<core::Actor>();
                logger->info("Player collided with actor {}", other_actor.id);
            }
        });

    world.system<physics::CollisionEnded, PlayerTest>("PlayerCollisionEnded")
        .term_at(0).second(flecs::Wildcard)
        .kind(flecs::OnUpdate)
        .each([&, logger](flecs::iter& it, size_t, physics::CollisionEnded, PlayerTest) {
            logger->info("Player stopped colliding with something!");
            auto other = it.pair(0).second();

            if (other.has<core::Actor>()) {
                const auto& other_actor = other.get<core::Actor>();
                logger->info("Player stopped colliding with actor {}", other_actor.id);
            }
        });

    const auto& camera = world.get<rendering::Camera>();
    auto tick_source = world.lookup("core::Core::TickSource");

    world.component<GlobalActions>().add(flecs::Sparse);
    world.emplace<GlobalActions>(test_actor, tick_source, camera);
    auto& global_actions = world.get_mut<GlobalActions>();

    auto& action_router = world.get_mut<input::ActionRouter>();

    chain::Connect(action_router.global_action_producer, global_actions);
    
    // auto& build_input = world.get_mut<input::BuildInput>();
    // auto& room_input = world.get_mut<input::RoomInput>();
    // auto& input_state = world.get_mut<input::InputState>();
    
    // chain::Connect(input_state.idle_actions, global_actions);
    // chain::Connect(build_input.uncaptured_actions, global_actions);
    // chain::Connect(room_input.uncaptured_actions, global_actions);

    world.system()
        .tick_source(tick_source)
        .run([logger](flecs::iter&) {
            logger->info("Ticking...! Time: {}", TickClock::now().time_since_epoch().count());
        });
}