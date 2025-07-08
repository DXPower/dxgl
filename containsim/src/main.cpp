#include <dxgl/Texture.hpp>
#include <exception>
#include <iostream>

#include <flecs.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <dxgl/Application.hpp>
#include <dxgl/Screenbuffer.hpp>
#include <dxgl/Ubo.hpp>

#include <components/Transform.hpp>
#include <common/Logging.hpp>
#include <modules/rendering/Camera.hpp>
#include <systems/TilePrefabs.hpp>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <systems/CircleMover.hpp>
#include <components/Actor.hpp>
#include <components/Mobility.hpp>
#include <common/DebugDraws.hpp>
#include <spdlog/spdlog.h>

#include <modules/application/Application.hpp>
#include <modules/input/Input.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/rendering/Rendering.hpp>
#include <modules/rendering/SpriteRenderer.hpp>
#include <modules/rendering/TileGridRenderer.hpp>
#include <modules/rendering/RoomRenderer.hpp>
#include <modules/core/Core.hpp>
#include <modules/ui/Ui.hpp>
#include <modules/experiment/Experiment.hpp>

using namespace dxgl;

struct InputResults {
    glm::vec2 camera_movement{};
};

static InputResults ProcessInput(GLFWwindow* window) {
    InputResults result{};

    auto IsKeyPressed = [window](int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    };

    // if (IsKeyPressed(GLFW_KEY_ESCAPE))
    //     glfwSetWindowShouldClose(window, 1);
    
    if (IsKeyPressed(GLFW_KEY_W))
        result.camera_movement.y -= 1;
    if (IsKeyPressed(GLFW_KEY_S))
        result.camera_movement.y += 1;
    if (IsKeyPressed(GLFW_KEY_A))
        result.camera_movement.x -= 1;
    if (IsKeyPressed(GLFW_KEY_D))
        result.camera_movement.x += 1;

    if (result.camera_movement != glm::vec2(0, 0))
        result.camera_movement = glm::normalize(result.camera_movement);

    return result;
}


static void Clear() {
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // NOLINT
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

#define CATCH_EXCEPTIONS 1

int main() {
    logging::SetCommonSink(logging::CreateConsoleSink());

    auto logger = logging::CreateLogger("main");
    logger.set_level(spdlog::level::debug);

#if CATCH_EXCEPTIONS == 1
    try {
#endif        
        flecs::world world{};

        world.import<application::Application>();
        auto main_window_e = world.query<application::MainWindow>().first();
        auto& main_window = main_window_e.get_mut<dxgl::Window>();

        world.import<core::Core>();

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.Resize(main_window.GetSize());

        services::InitTilePrefabs(world);

        world.import<rendering::Rendering>();
        auto& camera = world.get_mut<rendering::Camera>();
        auto& ubos = world.get_mut<dxgl::UboBindingManager>();
        auto& draw_queues = world.get_mut<rendering::DrawQueues>();

        using namespace components;
        
        DebugDraws::Init(ubos, draw_queues);

        world.observer<const application::WindowSize>()
            .term_at(0).src(main_window_e)
            .event(flecs::OnSet)
            .each([&](const application::WindowSize& size) {
                main_screen_buffer.Resize(size.value);
            });

        double last_time{};
        constexpr float camera_speed = 350.f;

        world.import<input::Input>();
        world.import<ui::Ui>();
        world.import<physics::Physics>();
        world.import<pathing::Pathing>();
        world.import<experiment::Experiment>();

        auto* rml_context = world.query<application::RmlMainContext>()
            .first().get<application::RmlContextHandle>().context;

        auto& rml_renderer = dynamic_cast<RenderInterface_GL3&>(
            world.get_mut<application::UiEnv>().GetRenderInterface()
        );
        std::ios_base::sync_with_stdio(false);
        // float last_delta_time = 0.0f;
        while (!main_window.ShouldClose()) {
            double current_time = Application::GetTime();
            double delta_time_d = current_time - last_time;
            
            last_time = current_time;
            auto delta_time = static_cast<float>(delta_time_d);

            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);
            draw_queues = {};

            auto input [[maybe_unused]] = ProcessInput(main_window.GetGlfwWindow());
            camera.MoveBy(input.camera_movement * camera_speed * delta_time);

            world.progress();

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_screen_buffer.Render();

            dxgl::Screenbuffer::Unuse();

            rml_context->Update();
            rml_renderer.BeginFrame();
            rml_context->Render();
            rml_renderer.EndFrame();

            main_window.SwapBuffers();
            main_window.PollEvents();
        }

        Rml::Shutdown();
        Application::Terminate();
#if CATCH_EXCEPTIONS == 1
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
#endif    

}
