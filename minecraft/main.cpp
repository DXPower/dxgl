#include <exception>
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
#include <dxgl/Camera.hpp>
#include <dxgl/Cube.hpp>
#include <dxgl/Cubemap.hpp>
#include <dxgl/Framebuffer.hpp>
#include <dxgl/Light.hpp>
#include <dxgl/Graphics.hpp>
#include <dxgl/Material.hpp>
#include <dxgl/Mesh.hpp>
#include <dxgl/Screenbuffer.hpp>
#include <dxgl/Skybox.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Renderbuffer.hpp>
#include <dxgl/Uniform.hpp>
#include <dxgl/Ubo.hpp>
#include <dxgl/Vao.hpp>
#include <dxgl/Vbo.hpp>

using namespace dxgl;

void ProcessInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void OnInput(GLFWwindow* window [[maybe_unused]], int key, int scancode [[maybe_unused]], int action, int mods [[maybe_unused]]) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_UP:
                break;
        }
    }
}

std::pair<Vao, Vbo> MakeTri() {
    std::vector<float> vertices = {
        0, 0.75, 0.5,
        -0.5, 0, 0.5,
        0.5, 0, 0.5
    };

    Vao vao{};
    vao.Use();

    Vbo vbo{};
    vbo.Upload(vertices);

    VaoAttribBuilder()
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(3)
            )
        )
        .Apply(vao, vbo);

    return { std::move(vao), std::move(vbo) };
}

void Clear() {
    glClearColor(0.1, 0.2, 0.4, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


int main() {
    Application::Init("Minecraft2D", 1000, 800);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(Application::GetWindow(), OnInput);

    
    Screenbuffer main_screen_buffer{};
    main_screen_buffer.ResizeToScreen();

    Application::OnWindowResize([&main_screen_buffer](int, int) {
        main_screen_buffer.ResizeToScreen();
    });

    auto basic_program = ProgramBuilder()
        .Vert("shaders/basic.vert")
        .Frag("shaders/basic.frag")
        .Link();

    auto [tri_vao, tri_vbo] = MakeTri();

    float last_time = 0.0f;
    float delta_time [[maybe_unused]] = 0.0f;

    while (!Application::ShouldQuit()) {
        main_screen_buffer.Use();
        Clear();
        glEnable(GL_DEPTH_TEST);

        float current_time = Application::GetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        ProcessInput(Application::GetWindow());

        basic_program.Use();
        tri_vao.Use();

        glDrawArrays(GL_TRIANGLES, 0, 3);
    
        main_screen_buffer.Render();

        Application::SwapBuffers();
        Application::PollEvents();
    }

    Application::Destroy();
}