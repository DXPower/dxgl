#include <dxgl/Application.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

using namespace dxgl;

namespace {
    GLFWwindow* window{};
    std::function<OnWindowResizeFunc> on_window_resize{};

    void InitGlfw() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    void InitGlad() {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }
    }

    void OnWindowResizeImpl(GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
        
        if (on_window_resize)
            on_window_resize(width, height);
    }
}

void Application::Init(dxtl::cstring_view title, int screenW, int screenH) {
    InitGlfw();

    window = glfwCreateWindow(screenW, screenH, title.c_str(), nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, OnWindowResizeImpl);

    InitGlad();
    glViewport(0, 0, screenW, screenH);
}

void Application::OnWindowResize(std::function<OnWindowResizeFunc> func) {
    on_window_resize = std::move(func);
}

void Application::SwapBuffers() {
    glfwSwapBuffers(window);
}

void Application::PollEvents() {
    glfwPollEvents();
}

void Application::Destroy() {
    glfwTerminate();
}

double Application::GetTime() {
    return glfwGetTime();
}

glm::vec<2, int> Application::GetWindowSize() {
    glm::vec<2, int> size;
    glfwGetWindowSize(window, &size.x, &size.y);

    return size;
}


bool Application::ShouldQuit() {
    return glfwWindowShouldClose(window);
}

GLFWwindow* Application::GetWindow() {
    return window;
}
