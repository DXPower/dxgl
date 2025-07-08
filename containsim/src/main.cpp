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

static bool toggle_debugger = false;
static std::optional<glm::vec2> target_pos{};

struct GlobalActions : ActionConsumer {
    void Consume(Action&& action) override {
        const KeyPress* press = std::get_if<KeyPress>(&action.data);
        if (press != nullptr && press->IsDownKey(GLFW_KEY_F8)) {
            toggle_debugger = true;
        }

        const MouseClick* click = std::get_if<MouseClick>(&action.data);
        if (click != nullptr && click->button == 0 && click->dir == ButtonDir::Up) {
            target_pos = click->pos;
        }
    }
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
    logger.info("Hello, world!");
    logger.set_level(spdlog::level::debug);

#if CATCH_EXCEPTIONS == 1
    try {
#endif        
        flecs::world world{};

        world.import<application::Application>();
        auto main_window_e = world.query<application::MainWindow>().first();
        auto& main_window = main_window_e.get_mut<dxgl::Window>();

        if (!Rml::LoadFontFace("res/fonts/LatoLatin-Regular.ttf", true)) {
            throw std::runtime_error("Failed to load RmlUi font face");
        }

        world.import<core::Core>();

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.Resize(main_window.GetSize());

        // auto& tile_grid = world.get_mut<core::TileGrid>();

        services::InitTilePrefabs(world);

        world.import<rendering::Rendering>();
        auto& camera = world.get_mut<rendering::Camera>();
        auto& ubos = world.get_mut<dxgl::UboBindingManager>();
        auto& draw_queues = world.get_mut<rendering::DrawQueues>();

        camera.UpdateViewportSize(main_window.GetSize());

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

        auto& build_input = world.get_mut<input::BuildInput>();
        auto& room_input = world.get_mut<input::RoomInput>();
        auto& input_state = world.get_mut<input::InputState>();

        GlobalActions global_actions{};

        chain::Connect(input_state.idle_actions, global_actions);
        chain::Connect(build_input.uncaptured_actions, global_actions);
        chain::Connect(room_input.uncaptured_actions, global_actions);

        world.import<physics::Physics>();
        world.import<pathing::Pathing>();
        world.import<experiment::Experiment>();

        auto test_actor = world.lookup("experiment::Experiment::TestActor");
        assert(test_actor.is_valid());

        auto* rml_context = world.query<application::RmlMainContext>()
            .first().get<application::RmlContextHandle>().context;

        auto& rml_renderer = dynamic_cast<RenderInterface_GL3&>(
            world.get_mut<application::UiEnv>().GetRenderInterface()
        );

        float last_delta_time = 0.0f;
        while (!main_window.ShouldClose()) {
            double current_time = Application::GetTime();
            double delta_time_d = current_time - last_time;
            
            constexpr auto fps_60 = 1.0 / 60.0;
            if (last_delta_time != 0 && delta_time_d < (fps_60 - last_delta_time)) {
                if (delta_time_d < (fps_60 / 4)) {
                    std::this_thread::sleep_for(std::chrono::microseconds((long long)((fps_60 - delta_time_d) / 2) * 1000));
                }

                continue;
            }
            
            last_time = current_time;
            float delta_time = static_cast<float>(delta_time_d);
            last_delta_time = delta_time;
            
            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);
            draw_queues = {};

            auto input [[maybe_unused]] = ProcessInput(main_window.GetGlfwWindow());

            if (target_pos.has_value()) {
                const auto cam_view = camera.GetViewMatrix();
                const auto world_pos = glm::inverse(cam_view) * glm::vec4(*target_pos, 1, 1);

                logger.info("Moving to {}, {}", world_pos.x, world_pos.y);

                test_actor.set(pathing::DestinationIntent{.position = world_pos});
                test_actor.add<pathing::StaleDestination>();
                target_pos.reset();
            }

            camera.MoveBy(input.camera_movement * camera_speed * delta_time);

            assert(main_window_e.has<application::WindowSize>());
            world.progress();


            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_screen_buffer.Render();

            dxgl::Screenbuffer::Unuse();

            if (toggle_debugger) {
                toggle_debugger = false;
                Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
            }

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
