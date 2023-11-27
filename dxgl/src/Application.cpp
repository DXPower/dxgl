#include <dxgl/Application.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <stdexcept>

using namespace dxgl;

namespace {
    // std::unordered_map<GLFWwindow*, std::function<OnWindowResizeFunc>> window_resize_funcs{};

    void InitGlfw() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
}

Window::Window(dxtl::cstring_view title, glm::ivec2 window_size, const Window* share) {
    glfw_window.reset(
        glfwCreateWindow(
            window_size.x,
            window_size.y,
            title.c_str(),
            nullptr,
            share ? share->GetGlfwWindow() : nullptr
        )
    );

    if (glfw_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetFramebufferSizeCallback(GetGlfwWindow(), OnWindowResizeImpl);
    glfwSetWindowUserPointer(GetGlfwWindow(), this);
}

Window::Window(dxtl::cstring_view, Fullscreen, const Window*) {
    throw std::runtime_error("Unimplemented");
}
    
void Window::GlfwWindowDeleter::operator()(GLFWwindow* ptr) const {
    glfwDestroyWindow(ptr);
}

void Window::OnResize(std::function<OnWindowResizeFunc> func) {
    // window_resize_funcs.insert_or_assign(GetGlfwWindow(), std::move(func));
    resize_func = std::move(func);
}

void Window::MakeCurrent() const {
    glfwMakeContextCurrent(GetGlfwWindow());
}

void Window::SwapBuffers() {
    glfwSwapBuffers(GetGlfwWindow());
}

void Window::PollEvents() {
    glfwPollEvents();
}

glm::ivec2 Window::GetSize() const {
    glm::ivec2 size;
    glfwGetWindowSize(GetGlfwWindow(), &size.x, &size.y);

    return size;
}

glm::dvec2 Window::GetMousePos() const {
    double x, y;
    glfwGetCursorPos(GetGlfwWindow(), &x, &y);

    return {x, y};
}

glm::vec2 Window::GetScale() const {
    glm::vec2 scale{};
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &scale.x, &scale.y);
    return scale;
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(GetGlfwWindow());
}

void Window::OnWindowResizeImpl(GLFWwindow* glfw_window, int width, int height) {
    glfwMakeContextCurrent(glfw_window);
    glViewport(0, 0, width, height);
    
    Window& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
    window.resize_func({width, height});
}

void Application::Init() {
    InitGlfw();
    // InitGlad();
}

void Application::Terminate() {
    glfwTerminate();
}

double Application::GetTime() {
    return glfwGetTime();
}
