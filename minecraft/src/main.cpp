#include <exception>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <format>
#include <concepts>
#include <numbers>
#include <ranges>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <dxgl/Application.hpp>
#include <dxgl/Framebuffer.hpp>
#include <dxgl/Graphics.hpp>
#include <dxgl/Screenbuffer.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Uniform.hpp>
#include <dxgl/Ubo.hpp>
#include <dxgl/Vao.hpp>

#include "Block.hpp"
#include "Camera.hpp"
#include "GlobalState.hpp"
#include "Player.hpp"
#include "Physics.hpp"

using namespace dxgl;

struct InputResults {
    glm::vec2 player_movement{};
};

InputResults ProcessInput(GLFWwindow* window) {
    InputResults result{};

    auto IsKeyPressed = [window](int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    };

    if (IsKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
    
    if (IsKeyPressed(GLFW_KEY_W))
        result.player_movement.y -= 1;
    if (IsKeyPressed(GLFW_KEY_S))
        result.player_movement.y += 1;
    if (IsKeyPressed(GLFW_KEY_A))
        result.player_movement.x -= 1;
    if (IsKeyPressed(GLFW_KEY_D))
        result.player_movement.x += 1;

    if (result.player_movement != glm::vec2(0, 0))
        result.player_movement = glm::normalize(result.player_movement);

    return result;
}

void OnInput(GLFWwindow* window [[maybe_unused]], int key, int scancode [[maybe_unused]], int action, int mods [[maybe_unused]]) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_UP:
                break;
        }
    }
}

void Clear() {
    glClearColor(0.1, 0.2, 0.4, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

struct CameraUbo {
    glm::mat4 camera_matrix;
};

BOOST_DESCRIBE_STRUCT(CameraUbo, (), (camera_matrix));

int main() {
    constexpr glm::vec2 initial_screen_size = { 1000, 800 };
    try {
        Application::Init("Minecraft2D", initial_screen_size.x, initial_screen_size.y);

        glfwSetKeyCallback(Application::GetWindow(), OnInput);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.ResizeToScreen();

        GlobalState global_state{};
        global_state.debug_draws.Init(global_state);

        Camera camera(global_state);
        camera.UpdateViewportSize(initial_screen_size.x, initial_screen_size.y);


        Application::OnWindowResize([&main_screen_buffer, &camera](int x, int y) {
            main_screen_buffer.ResizeToScreen();
            camera.UpdateViewportSize(x, y);
        });

        Chunk::InitDraw(global_state);
        Player::InitDraw(global_state);

        Chunk chunk{};
        chunk.blocks[0].type = BlockType::GRASS_TOP;
        chunk.blocks[1].type = BlockType::STONE;
        chunk.blocks[2].type = BlockType::DIRT;
        chunk.blocks[3].type = BlockType::GRASS;
        chunk.blocks[4].type = BlockType::WOOD;
        chunk.blocks[5].type = BlockType::LAVA;

        Player player{};
        player.SetPosition({500, 400});
        const float player_speed = 300.f;

        float last_time = 0.0f;
        float delta_time = 0.0f;
        
        while (!Application::ShouldQuit()) {
            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);

            float current_time = Application::GetTime();
            delta_time = current_time - last_time;
            last_time = current_time;

            auto input = ProcessInput(Application::GetWindow());

            player.SetVelocity(input.player_movement * player_speed);
            player.Update(delta_time);

            camera.LookAt(player.GetPosition());
            // camera.MoveBy(input.camera_movement * delta_time * camera_speed);
            // std::cout << camera.GetPosition().x << "\t" << camera.GetPosition().y << '\n';

            chunk.Render();
            player.Render();

            using Physics::Aabb;
            using Physics::Line;
            // auto block_aabb = Aabb{
            //         .position = camera.GetViewMatrix() * glm::vec4(chunk.blocks[0].position, 1, 1),
            //         .size = Chunk::block_size
            // };

            // auto near_far = Physics::GetNearFarPoints(block_aabb, Line{
            //     .from = screen_center,
            //     .to = mouse_pos
            // });

            // auto line_collision = Physics::TestAabbLineCollision(block_aabb, Line{
            //     .from = screen_center,
            //     .to = mouse_pos
            // });

            // global_state.debug_draws.Draw(DebugArrow{
            //     .from = screen_center,
            //     .to = mouse_pos
            // }, glm::vec4{1, 0, 0, 1});

            // global_state.debug_draws.Draw(DebugHash{
            //     .position = near_far.near_x,
            //     .radius = 5
            // }, glm::vec4{1, 1, 0, 1});

            // global_state.debug_draws.Draw(DebugHash{
            //     .position = near_far.far_x,
            //     .radius = 5
            // }, glm::vec4{1, 0.5, 0, 1});

            // global_state.debug_draws.Draw(DebugHash{
            //     .position = near_far.near_y,
            //     .radius = 5
            // }, glm::vec4{1, 1, 0, 1});

            // global_state.debug_draws.Draw(DebugHash{
            //     .position = near_far.far_y,
            //     .radius = 5
            // }, glm::vec4{1, 0.5, 0, 1});

            // if (line_collision.has_value()) {
            //     global_state.debug_draws.Draw(DebugHash{
            //         .position = line_collision->hit_position,
            //         .radius = 10
            //     }, glm::vec4(1, 1, 1, 1));
            // }

            // auto collisions = Physics::TestAabbCollision(
            //     Aabb{.position = mouse_pos, .size = glm::vec2(75.f, 75.f)},
            //     block_aabb
            // );

            // if (collisions.has_value()) {
            //     global_state.debug_draws.Draw(DebugSquare{
            //         .position = glm::vec2(mouse_pos) - (*collisions)[0].penetration,
            //         .size = glm::vec2(75.f, 75.f)
            //     }, glm::vec4(0, 0, 1, 1));

            //     global_state.debug_draws.Draw(DebugSquare{
            //         .position = glm::vec2(mouse_pos) - (*collisions)[1].penetration,
            //         .size = glm::vec2(75.f, 75.f)
            //     }, glm::vec4(0, 0, 1, 1));
            // }

            // global_state.debug_draws.Draw(DebugSquare{
            //     .position = mouse_pos,
            //     .size = {75.f, 75.f}
            // }, {1, 0, 0, 1});
            // global_state.debug_draws.Draw(DebugArrow{
            //     .from = Application::GetWindowSize() / 2,
            //     .to = camera.GetViewMatrix() * glm::vec4(chunk.blocks[0].position, 1, 1)
            // }, glm::vec4(1, 0, 0, 1));

            // auto screen_center = Application::GetWindowSize() / 2;
            auto mouse_pos = Application::GetMousePos();
            glm::vec2 mouse_world_pos = glm::inverse(camera.GetViewMatrix()) * glm::vec4(mouse_pos, 1, 1);

            auto moved = Aabb{
                .position = mouse_world_pos,
                .size = {75.f, 75.f}
            };

            global_state.debug_draws.Draw(DebugSquare{
                .position = camera.GetViewMatrix() * glm::vec4(moved.position, 1, 1),
                .size = moved.size
            }, glm::vec4(1, 0, 0, 1));

            global_state.debug_draws.Draw(DebugSquare{
                .position = camera.GetViewMatrix() * glm::vec4(moved.position + glm::vec2(-200, 100), 1, 1),
                .size = moved.size
            }, glm::vec4(1, 1, 0, 1));

            auto colls = Physics::SweepChunk(moved, glm::vec2(-200, 100), chunk);

            for (const auto& block : chunk.blocks) {
                    global_state.debug_draws.Draw(DebugSquare{
                        .position = camera.GetViewMatrix() * glm::vec4(chunk.GetCoordPos(block.rel_coord), 1, 1),
                        .size = {100.f, 100.f}
                    }, glm::vec4(0.5, 1, 1, 1));
            }

            bool first = true;
            for (const auto& col : colls) {
                    global_state.debug_draws.Draw(DebugSquare{
                        .position = camera.GetViewMatrix() * glm::vec4(col.final_position, 1, 1),
                        .size = {75.f, 75.f}
                    }, first ? glm::vec4(0, 1, 0, 1) : glm::vec4(0, 1, 0, 0.3f));

                    first = false;
            }

            auto chunk_col = Physics::ResolveAgainstChunk(moved, glm::vec2(-200, 100), chunk);

            if (chunk_col.has_value()) {
                global_state.debug_draws.Draw(DebugSquare{
                    .position = camera.GetViewMatrix() * glm::vec4(chunk_col->final_position, 1, 1),
                    .size = {75.f, 75.f}
                }, glm::vec4(1, .3, 0.2, 1));
            }



            // auto sweep = Physics::SweepAabbCollision(moved, glm::vec2(-200, 100), block_aabb);

            // if (sweep.has_value()) {
            //     global_state.debug_draws.Draw(DebugSquare{
            //         .position = sweep->final_position,
            //         .size = {75.f, 75.f}
            //     }, glm::vec4(0, 1, 0, 1));

            //     global_state.debug_draws.Draw(DebugArrow{
            //         .from = sweep->hit_position,
            //         .to = sweep->hit_position + sweep->surface_normal * 25.f
            //     }, glm::vec4(0, 1, 0, 1));
            // }

            global_state.debug_draws.Render();
            main_screen_buffer.Render();

            Application::SwapBuffers();
            Application::PollEvents();
        }

        Application::Destroy();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}