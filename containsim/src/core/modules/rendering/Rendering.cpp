#include <glad/glad.h>
#include <modules/rendering/Rendering.hpp>
#include <modules/application/Application.hpp>

#include <modules/rendering/SpriteRenderer.hpp>
#include <modules/rendering/TileGridRenderer.hpp>
#include <modules/rendering/RoomRenderer.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/Camera.hpp>
#include <modules/rendering/DebugDraws.hpp>

#include <components/Transform.hpp>
#include <dxgl/Ubo.hpp>

using namespace rendering;


namespace {
void PrepareForNextFrame(dxgl::Screenbuffer& main_screen_buffer) {
    main_screen_buffer.Use();
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // NOLINT
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
}
}

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
    const auto& main_window = main_window_e.get<dxgl::Window>();
    
    // Setup screenbuffer
    world.component<dxgl::Screenbuffer>();
    auto screenbuffer_e = world.entity()
        .add<dxgl::Screenbuffer>()
        .add<MainScreenbuffer>();

    screenbuffer_e.get_mut<dxgl::Screenbuffer>().Resize(main_window.GetSize());

    world.observer<application::WindowSize, dxgl::Screenbuffer>()
        .term_at<application::WindowSize>().src(main_window_e)
        .term_at<dxgl::Screenbuffer>().filter().src(screenbuffer_e)
        .event(flecs::OnSet)
        .each([](const application::WindowSize& size, dxgl::Screenbuffer& screenbuffer) {
            screenbuffer.Resize(size.value);
        });

    world.system<dxgl::Screenbuffer>("PrepareForNextFrame")
        .with<MainScreenbuffer>()
        .kind(flecs::PreUpdate)
        .each(&PrepareForNextFrame);

    // Setup camera
    auto& camera = world.get_mut<Camera>();
    camera.UpdateViewportSize(main_window.GetSize());

    world.observer<application::WindowSize, Camera>()
        .term_at<application::WindowSize>().src(main_window_e)
        .term_at<Camera>().filter().singleton()
        .event(flecs::OnSet)
        .each([](const application::WindowSize& size, Camera& camera) {
            camera.UpdateViewportSize(size.value);
        });

    SpriteRendererSystems(world);
    TileGridRendererSystem(world);
    RoomRendererSystem(world);

    world.component<DebugDraws>();
    world.emplace<DebugDraws>(ubos);

    // Clear the main draw queues at the start of each frame
    world.system<DrawQueues>("ClearDrawQueues")
        .term_at<DrawQueues>().singleton()
        .kind(flecs::PreUpdate)
        .each([](DrawQueues& queues) {
            queues.ClearQueuedDraws();
        });

    // Custom rendering phase to render the draw queues and then the final screenbuffer
    auto world_render_e = world.entity("WorldRenderPhase")
        .add(flecs::Phase)
        .add(flecs::DependsOn, flecs::OnStore);

    // Mark the final presentation as dependent on this phase
    world.lookup("application::Application::FinalPresentPhase")
        .add(flecs::DependsOn, world_render_e);

    world.system<const DrawQueues>("RenderQueuedDraws")
        .term_at(0).singleton()
        .kind(world_render_e)
        .each([](const DrawQueues& queues) {
            queues.RenderQueuedDraws();
        });

    world.system<const dxgl::Screenbuffer>("RenderScreenbuffer")
        .with<MainScreenbuffer>()
        .kind(world_render_e)
        .each([](const dxgl::Screenbuffer& screenbuffer) {
            screenbuffer.Render();
        });
}