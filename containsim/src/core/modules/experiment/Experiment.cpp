#include <modules/experiment/Experiment.hpp>

#include <modules/core/Core.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/rendering/Rendering.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/input/Input.hpp>
#include <modules/research/Research.hpp>
#include <modules/prefabs/Prefabs.hpp>

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

        // const MouseClick* click = std::get_if<MouseClick>(&action.data);
        // if (click != nullptr && click->button == 0 && click->dir == ButtonDir::Up) {
        //     // target_pos = click->pos;
        //     const auto cam_view = cam->GetViewMatrix();
        //     const auto world_pos = glm::inverse(cam_view) * glm::vec4(click->pos, 1, 1);

        //     target.set(pathing::DestinationIntent{.position = world_pos});
        //     target.add<pathing::StaleDestination>();
        // }

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
    world.import<research::Research>();
    world.import<prefabs::Prefabs>();

    auto logger = logging::CreateSharedLogger("Experiment");

    const auto& tile_grid = world.get<core::TileGrid>();
    auto& build_manager = world.get_mut<core::BuildManager>();

    const auto tile_sel = TileSelection{
        .start = {0, 0},
        .end = tile_grid.GetGridSize() - glm::ivec2{1, 1}
    };

    const auto& tile_type_metas = world.get<core::TileTypeMetas>();
    const TileType grass = tile_type_metas.GetIdOfType("grass").value();

    for (auto tile_coord : tile_sel.Iterate()) {
        build_manager.PlaceTile(tile_coord, grass);
    }

    // Populate world actors
    auto test_actor = world.entity()
        .is_a<prefabs::Researcher>()
        .set_name("TestResearcher");
    test_actor.get_mut<Transform>().position = {400, 400};

    auto desk1 = world.entity()
        .is_a<prefabs::ResearchDesk>()
        .set_name("desk1");
    desk1.get_mut<Transform>().position = {1300, 200};

    auto desk2 = world.entity()
        .is_a<prefabs::ResearchDesk>()
        .set_name("desk2");
    desk2.get_mut<Transform>().position = {1200, 400};

    const auto& camera = world.get<rendering::Camera>();
    auto tick_source = world.lookup("core::Core::TickSource");

    world.component<GlobalActions>().add(flecs::Sparse);
    world.emplace<GlobalActions>(test_actor, tick_source, camera);
    auto& global_actions = world.get_mut<GlobalActions>();

    auto& action_router = world.get_mut<input::ActionRouter>();

    chain::Connect(action_router.global_action_producer, global_actions);

}