#include <common/GlobalConfig.hpp>
#include <dxgl/Texture.hpp>
#include <exception>
#include <iostream>

#include <flecs.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <dxgl/Application.hpp>
#include <dxgl/Screenbuffer.hpp>

#include <common/DrawQueues.hpp>
#include <components/RenderData.hpp>
#include <components/Sprite.hpp>
#include <components/Transform.hpp>
#include <systems/SpriteRenderer.hpp>

#include <services/TileGrid.hpp>
#include <services/TileGridRenderer.hpp>

#include <services/UiContainer.hpp>
#include <services/InputHandler.hpp>
#include <services/ActionRouter.hpp>
#include <services/Logging.hpp>

#include <common/GlobalData.hpp>
#include "Camera.hpp"

#include <spdlog/spdlog.h>

using namespace dxgl;

struct InputResults {
    glm::vec2 camera_movement{};
};

InputResults ProcessInput(GLFWwindow* window) {
    InputResults result{};

    auto IsKeyPressed = [window](int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    };

    if (IsKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, 1);
    
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

bool cycle_tiles = false;

void Clear() {
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // NOLINT
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

int main() {
    namespace logging = services::logging;
    logging::SetCommonSink(logging::CreateConsoleSink());

    constexpr glm::ivec2 initial_screen_size = { 1000, 800 };
    constexpr glm::ivec2 initial_debug_screen_size = { 800, 800 };
    try {
        Application::Init();
        Window main_window("Containment Simulator", initial_screen_size);
        main_window.MakeCurrent();
        
        if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, initial_screen_size.x, initial_screen_size.y);

        Window debug_window("Containment Simulator Debug", initial_debug_screen_size, &main_window);
        debug_window.MakeCurrent();
        glViewport(0, 0, initial_debug_screen_size.x, initial_debug_screen_size.y);

        main_window.MakeCurrent();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.Resize(initial_screen_size);

        GlobalState global_state{};

        Camera camera(global_state);
        camera.UpdateViewportSize(initial_screen_size);

        flecs::world world{};

        using namespace components;
        DrawQueues draw_queues{};
        systems::SpriteRenderer sprite_renderer(global_state, draw_queues);

        std::function<void(
            const components::SpriteRenderer&,
            const components::Transform&,
            const components::RenderData&,
            const components::Sprite&
        )> pre_store = std::bind_front(&systems::SpriteRenderer::PreStore, &sprite_renderer);

        world.system<
            components::SpriteRenderer,
            components::Transform,
            components::RenderData,
            components::Sprite>(
            "SpriteRenderer"
        )
            .kind(flecs::PreStore)
            .each(pre_store);
        
        GlobalConfig global_config{
            .map_size = { 20, 10 }, // NOLINT
            .tile_size = {100, 100} // NOLINT
        };

        world.set(GlobalData{
            .config = &global_config,
            .state = &global_state,
            .draw_queues = &draw_queues
        });

        services::TileGrid tile_grid(global_config);
        services::TileGridRenderer tile_grid_renderer(tile_grid, global_state.ubo_manager);

        auto logger = services::logging::CreateLogger("main");
        logger.info("Hello, world!");
        logger.set_level(spdlog::level::debug);

        auto SetTiles = [&, start = 0]() mutable {
            logger.debug("start: {}", start);

            for (int x = 0; x < global_config.map_size.x; x++) {
                for (int y = 0; y < global_config.map_size.y; y++) {
                    TileData data{};
                    data.type = static_cast<TileType>(start);
                    tile_grid.SetTile({x, y}, data);
                    
                    start = (int) ((start + 1) % magic_enum::enum_count<TileType>());
                }
            }
        };

        SetTiles();

        services::UiContainer ui_container(main_window, debug_window);
        ui_container.GetMainView().LoadUrl("file:///game/ingame.html");

        main_window.OnResize([&](glm::ivec2 size) {
            main_screen_buffer.Resize(size);
            camera.UpdateViewportSize(size);
            ui_container.GetMainView().Resize(size);
        });

        debug_window.OnResize([&](glm::ivec2 size) {
            ui_container.GetInspectorView().Resize(size);
        });

        float last_time{};
        constexpr float camera_speed = 350.f;

        services::InputHandler debug_input(debug_window);
        debug_input.OnAction([&](Action&& action) {
            // ui_renderer.InputActionDebug(action);
            ui_container.GetInspectorView().PushAction(std::move(action));
        });

        struct GameInput : services::IActionReceiver {
            void PushAction(Action&& action) override {
                if (std::holds_alternative<KeyPress>(action.data)) {
                    const auto& key_press = std::get<KeyPress>(action.data);

                    if (key_press.dir == ButtonDir::Down) {
                        if (key_press.key == GLFW_KEY_E) {
                            cycle_tiles = true;
                        }
                    }
                }
            }
        } game_receiver, offscreen_receiver;

        services::ActionRouter input_router(
            ui_container.GetMainView(), 
            game_receiver, 
            ui_container.GetMainView(),
            offscreen_receiver
        );
        
        services::InputHandler game_input(main_window);
        game_input.OnAction([&](Action&& action) {
            input_router.PushAction(std::move(action));
        });

        ui_container.GetMainView()
            .RegisterCallback(
                "OutputCommand",
                services::MakeUiCallback<std::string>([x = 0](std::string data) mutable {
                    std::cout << "Got OutputCommand " << x++ << std::endl;
                    std::cout << "Data: " << data << "\n=====" << std::endl;
                })
            );

        while (!main_window.ShouldClose() && !debug_window.ShouldClose()) {
            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);
            draw_queues = {};

            auto input [[maybe_unused]] = ProcessInput(main_window.GetGlfwWindow());
            float current_time = (float) Application::GetTime();
            float delta_time = current_time - last_time;
            last_time = current_time;

            camera.MoveBy(input.camera_movement * camera_speed * delta_time);

            world.progress();

            if (cycle_tiles) {
                SetTiles();
                cycle_tiles = false;
            }

            tile_grid_renderer.Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_screen_buffer.Render();

            // Update and render UI
            ui_container.Update();
            ui_container.Render();

            dxgl::Screenbuffer::Unuse();

            ui_container.GetMainView().Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_window.SwapBuffers();
            main_window.PollEvents();

            debug_window.MakeCurrent();
            Clear();

            ui_container.GetInspectorView().Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            debug_window.SwapBuffers();
            debug_window.PollEvents();

            main_window.MakeCurrent();
        }

        Application::Terminate();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
