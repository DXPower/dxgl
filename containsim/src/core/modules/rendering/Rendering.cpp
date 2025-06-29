#include <modules/rendering/Rendering.hpp>

#include <modules/rendering/SpriteRenderer.hpp>
#include <modules/rendering/TileGridRenderer.hpp>
#include <modules/rendering/RoomRenderer.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/Camera.hpp>
#include <components/Transform.hpp>
#include <dxgl/Ubo.hpp>

using namespace rendering;

Rendering::Rendering(flecs::world& world) {
    world.component<RenderData>();
    world.component<Sprite>();
    world.component<SpriteRenderer>();

    world.component<DrawQueues>().add(flecs::Sparse);
    world.add<DrawQueues>();

    world.component<dxgl::UboBindingManager>().add(flecs::Sparse);
    world.add<dxgl::UboBindingManager>();
    auto& ubos = world.get_mut<dxgl::UboBindingManager>();

    world.component<Camera>().add(flecs::Sparse);
    world.emplace<Camera>(ubos);

    SpriteRendererSystems(world);
    TileGridRendererSystem(world);
    RoomRendererSystem(world);
}