#include <modules/rendering/Rendering.hpp>
#include <modules/application/Application.hpp>

#include <modules/rendering/SpriteRenderer.hpp>
#include <modules/rendering/TileGridRenderer.hpp>
#include <modules/rendering/RoomRenderer.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/Camera.hpp>
#include <components/Transform.hpp>
#include <dxgl/Ubo.hpp>

using namespace rendering;

Rendering::Rendering(flecs::world& world) {
    world.import<application::Application>();

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

    auto main_window_e = world.query<application::MainWindow>().first();
    world.observer<application::WindowSize>()
        .term_at(0).src(main_window_e)
        .event(flecs::OnSet)
        .each([](flecs::iter& it, size_t, const application::WindowSize& size) {
            auto& camera = it.world().get_mut<Camera>();
            camera.UpdateViewportSize(size.value);
        });

    SpriteRendererSystems(world);
    TileGridRendererSystem(world);
    RoomRendererSystem(world);
}