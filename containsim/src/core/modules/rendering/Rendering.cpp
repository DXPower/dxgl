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
    auto& draw_queues = world.get_mut<DrawQueues>();

    world.component<dxgl::UboBindingManager>().add(flecs::Sparse);
    world.add<dxgl::UboBindingManager>();
    auto& ubos = world.get_mut<dxgl::UboBindingManager>();

    world.component<Camera>().add(flecs::Sparse);
    world.emplace<Camera>(ubos);

    world.component<SpriteRendererSystem>().add(flecs::Sparse);
    world.emplace<SpriteRendererSystem>(ubos, draw_queues);
    auto& sprite_renderer = world.get_mut<SpriteRendererSystem>();

    using components::Transform;
    
    world.system<SpriteRenderer, Transform, RenderData, Sprite>("SpriteRenderer")
        .kind(flecs::PreStore)
        .each(std::bind_front(&SpriteRendererSystem::PreStore, &sprite_renderer));

    const auto& tile_grid = world.get<services::TileGrid>();
    world.component<TileGridRenderer>().add(flecs::Sparse);
    world.emplace<TileGridRenderer>(tile_grid, ubos);

    const auto& room_manager = world.get<services::RoomManager>();
    world.component<RoomRenderer>().add(flecs::Sparse);
    world.emplace<RoomRenderer>(room_manager, ubos);
}