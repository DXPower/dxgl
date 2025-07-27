#include <modules/prefabs/Prefabs.hpp>

#include <modules/rendering/Rendering.hpp>
#include <modules/research/Research.hpp>
#include <modules/core/Core.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/pathing/Pathing.hpp>

using namespace prefabs;

Prefabs::Prefabs(flecs::world& world) {
    world.import<core::Core>();
    world.import<physics::Physics>();
    world.import<rendering::Rendering>();
    world.import<research::Research>();
    world.import<pathing::Pathing>();

    auto gardner_sheet_e = world.entity("GardnerSheet")
        .set<dxgl::Texture>(dxgl::LoadTextureFromFile("res/img/gardner.png"));

    const auto& gardner_sheet = gardner_sheet_e.get<dxgl::Texture>();

    world.prefab<Researcher>()
        .set<pathing::PathMover>(pathing::PathMover{})
        .set(core::Transform{
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
            .layer = rendering::RenderLayer::Objects
        })
        .add<rendering::SpriteRenderer>()
        .set<core::Mobility>(core::Mobility{.speed = 87.5f})
        .set(physics::Collider{
            .is_listening = true
        })
        .set(physics::SquareCollider{
            .relative_size = {.9f, .95f}
        })
        .add<ai::Performer>()
        .add<research::Researcher>();

    auto objects_sheet_e = world.entity("ObjectsSheet")
        .set<dxgl::Texture>(dxgl::LoadTextureFromFile("res/img/objects.png"));
    const auto& objects_sheet = objects_sheet_e.get<dxgl::Texture>();

    world.prefab<ResearchDesk>()
        .set(research::ResearchPoint{
            .science_per_success = 100
        })
        .set(core::Transform{
            .size = {64, 108}
        })
        .set(rendering::Sprite{
            .spritesheet = objects_sheet,
            .cutout = {
                .position = {895, 1870},
                .size = {64, 108}
            }
        })
        .set(rendering::RenderData{
            .layer = rendering::RenderLayer::Objects
        })
        .add<rendering::SpriteRenderer>()
        .set(physics::Collider{
            .is_trigger = true
        })
        .set(physics::SquareCollider{})
        .set(core::Interactable{
            .time_needed = TickDuration(16),
            .cooldown = TickDuration(16)
        });
}