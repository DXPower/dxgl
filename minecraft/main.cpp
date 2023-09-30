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

#include <dxgl/Camera.hpp>
#include <dxgl/Cube.hpp>
#include <dxgl/Cubemap.hpp>
#include <dxgl/Framebuffer.hpp>
#include <dxgl/Light.hpp>
#include <dxgl/Material.hpp>
#include <dxgl/Mesh.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Skybox.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Renderbuffer.hpp>
#include <dxgl/Uniform.hpp>
#include <dxgl/Ubo.hpp>
#include <dxgl/Vao.hpp>
#include <dxgl/Vbo.hpp>

using namespace dxgl;

struct ScreenFramebuffer {
    Framebuffer framebuffer{};
    Texture color_buffer{};
    Renderbuffer depth_stencil_buffer{};
};

std::unique_ptr<ScreenFramebuffer> MakeScreenFramebuffer(int w, int h) {
    auto screen = std::make_unique<ScreenFramebuffer>();

    screen->framebuffer.Use();
    screen->color_buffer.Use(0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen->color_buffer.GetHandle(), 0);

    screen->depth_stencil_buffer.Use();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screen->depth_stencil_buffer.GetHandle());

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    throw std::runtime_error("Framebuffer is not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    return screen;
}

std::pair<Vao, Vbo> MakeScreenVao() {
    float vert_data[] = {
        -1, 1, 0, 1,  // Top left
        -1, -1, 0, 0, // Bottom left
        1, -1, 1, 0,  // Bottom right
        1, 1, 1, 1,   // Top right
    };

    Vao vao{};
    vao.Use();

    Vbo vbo{};
    vbo.Upload(vert_data);

    VaoAttribBuilder()
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
            )
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
            )
        )
        .Apply(vao, vbo);

    return { std::move(vao), std::move(vbo) };
}

std::unique_ptr<ScreenFramebuffer> main_screen_buffer{};

void OnWindowResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
    Camera::Get().UpdateWindowSize(width, height);

    main_screen_buffer = MakeScreenFramebuffer(width, height);
}

float delta_time = 0.0f;

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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "Minecraft", nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, OnWindowResize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }  

    glViewport(0, 0, 800, 600);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, OnInput);

    Camera& camera = Camera::Get();
    camera.UpdateWindowSize(800, 600);

    auto basic_program = ProgramBuilder()
        .Vert("shaders/basic.vert")
        .Frag("shaders/basic.frag")
        .Link();

    auto [tri_vao, tri_vbo] = MakeTri();

    float last_time = 0.0f;

    main_screen_buffer = MakeScreenFramebuffer(800, 600);
    auto [screen_vao, screen_vbo] = MakeScreenVao();
    auto screen_program = ProgramBuilder()
        .Vert("shaders/framebuffer.vert")
        .Frag("shaders/framebuffer.frag")
        .Link();

    while (!glfwWindowShouldClose(window)) {
        main_screen_buffer->framebuffer.Use();
        Clear();
        glEnable(GL_DEPTH_TEST);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        ProcessInput(window);
        camera.UpdatePosition(window, delta_time);

        basic_program.Use();
        tri_vao.Use();

        glDrawArrays(GL_TRIANGLES, 0, 3);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.5, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        screen_program.Use();
        screen_vao.Use();
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        main_screen_buffer->color_buffer.Use(0);
        Uniform::Set(screen_program, "texture", 0);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}