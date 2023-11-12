#include <common/GlobalConfig.hpp>
#include <dxgl/Texture.hpp>
#include <exception>
#include <iostream>
#include <format>
#include <concepts>
#include <numbers>
#include <ranges>

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

#include <modules/TileGrid.hpp>

#include <common/GlobalData.hpp>
#include "Camera.hpp"


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
        glfwSetWindowShouldClose(window, true);
    
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

void OnInput(GLFWwindow* window [[maybe_unused]], int key, int scancode [[maybe_unused]], int action, int mods [[maybe_unused]]) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_E:
                cycle_tiles = true;
                break;
        }
    }
}

void Clear() {
    glClearColor(0.1, 0.2, 0.4, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

int main() {
    constexpr glm::vec2 initial_screen_size = { 1000, 800 };
    try {
        Application::Init("Containment Simulator", initial_screen_size.x, initial_screen_size.y);

        glfwSetKeyCallback(Application::GetWindow(), OnInput);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.ResizeToScreen();

        GlobalState global_state{};

        Camera camera(global_state);
        camera.UpdateViewportSize(initial_screen_size.x, initial_screen_size.y);

        Application::OnWindowResize([&main_screen_buffer, &camera](int x, int y) {
            main_screen_buffer.ResizeToScreen();
            camera.UpdateViewportSize(x, y);
        });

        dxgl::Texture spritesheet = dxgl::LoadTextureFromFile("res/img/tiles.png");
        spritesheet.SetFilterMode(dxgl::FilterMode::Nearest);


        flecs::world world{};

        using namespace components;
        DrawQueues draw_queues{};
        // systems::SpriteRenderer sprite_renderer(global_state, draw_queues);

        // std::function<void(
        //     const components::SpriteRenderer&,
        //     const components::Transform&,
        //     const components::RenderData&,
        //     const components::Sprite&
        // )> pre_store = std::bind_front(&systems::SpriteRenderer::PreStore, &sprite_renderer);

        // world.system<
        //     components::SpriteRenderer,
        //     components::Transform,
        //     components::RenderData,
        //     components::Sprite>(
        //     "SpriteRenderer"
        // )
        //     .kind(flecs::PreStore)
        //     .each(pre_store);

        // auto grass = world.entity("Square");

        // grass.set(components::Transform{
        //     .position = {700, 400},
        //     .size = {300, 300}
        // });

        // grass.set(components::RenderData{
        //     .layer = RenderLayer::Floors
        // });

        // grass.set(components::Sprite{
        //     .spritesheet = spritesheet,
        //     .cutout = {
        //         .position = {0, 140},
        //         .size = {28, 28}
        //     }
        // });

        // grass.add<components::SpriteRenderer>();
        
        // float last_time = 0.0f;
        // float delta_time [[maybe_unused]] = 0.0f;
        
        GlobalConfig global_config{
            .map_size = { 20, 10 },
            .tile_size = {100, 100}
        };

        world.set(GlobalData{
            .config = &global_config,
            .state = &global_state,
            .draw_queues = &draw_queues
        });

        auto tg = world.import<TileGrid::Module>();
        TileGrid::Module& tile_grid = *tg.get_mut<TileGrid::Module>();

        auto SetTiles = [&, start = 0]() mutable {
            for (int x = 0; x < global_config.map_size.x; x++) {
                for (int y = 0; y < global_config.map_size.y; y++) {
                    TileData data{};
                    data.type = static_cast<TileType>(start);
                    tile_grid.SetTile({x, y}, data);

                    start = (start + 1) % magic_enum::enum_count<TileType>();
                }
            }
        };

        SetTiles();

        float last_time{};
        constexpr float camera_speed = 350.f;

        while (!Application::ShouldQuit()) {
            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);
            draw_queues = {};

            auto input [[maybe_unused]] = ProcessInput(Application::GetWindow());
            float current_time = Application::GetTime();
            float delta_time = current_time - last_time;
            last_time = current_time;

            camera.MoveBy(input.camera_movement * camera_speed * delta_time);

            world.progress();

            if (cycle_tiles) {
                SetTiles();
                cycle_tiles = false;
            }

            tile_grid.Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_screen_buffer.Render();

            Application::SwapBuffers();
            Application::PollEvents();
        }

        Application::Destroy();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}