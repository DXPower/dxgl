#include <exception>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <format>
#include <concepts>
#include <numbers>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <xutility>

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

        Chunk chunk{
            .blocks = {
                Block{BlockType::GRASS_TOP},
                Block{BlockType::STONE},
                Block{BlockType::DIRT},
                Block{BlockType::GRASS},
                Block{BlockType::WOOD},
                Block{BlockType::LAVA}
            }
        };

        GlobalState global_state{};

        Camera camera(global_state);
        camera.UpdateViewportSize(initial_screen_size.x, initial_screen_size.y);

        const float camera_speed = 300.f;

        Chunk::InitDraw(global_state);

        Application::OnWindowResize([&main_screen_buffer, &camera](int x, int y) {
            main_screen_buffer.ResizeToScreen();
            camera.UpdateViewportSize(x, y);
        });

        
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
            camera.MoveBy(input.camera_movement * delta_time * camera_speed);
            // std::cout << camera.GetPosition().x << "\t" << camera.GetPosition().y << '\n';
        
            chunk.Render();

            main_screen_buffer.Render();

            Application::SwapBuffers();
            Application::PollEvents();
        }

        Application::Destroy();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}