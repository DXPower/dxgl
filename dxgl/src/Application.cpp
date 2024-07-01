#include <dxgl/Application.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <stdexcept>

using namespace dxgl;

namespace {
    // std::unordered_map<GLFWwindow*, std::function<OnWindowResizeFunc>> window_m_resize_funcs{};

    void InitGlfw() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
}

Window::Window(dxtl::cstring_view title, glm::ivec2 window_size)
    : Window(title, window_size, nullptr) { }

Window::Window(dxtl::cstring_view title, Fullscreen)
    : Window(title, Fullscreen{}, nullptr) { }

Window::Window(dxtl::cstring_view title, glm::ivec2 window_size, const Window* share) {
    m_glfw_window.reset(
        glfwCreateWindow(
            window_size.x,
            window_size.y,
            title.c_str(),
            nullptr,
            share ? share->GetGlfwWindow() : nullptr
        )
    );

    if (m_glfw_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetFramebufferSizeCallback(GetGlfwWindow(), OnWindowResizeImpl);
    glfwSetWindowUserPointer(GetGlfwWindow(), this);
}

Window::Window(dxtl::cstring_view, Fullscreen, const Window*) {
    throw std::runtime_error("Unimplemented");
}

Window::Window(Window&& move) noexcept
    : m_glfw_window(std::move(move.m_glfw_window))
    , m_resize_func(std::move(move.m_resize_func)) 
{
    glfwSetWindowUserPointer(GetGlfwWindow(), this);
}

Window& Window::operator=(Window&& move) noexcept {
    if (&move == this)
        return *this;

    std::swap(m_glfw_window, move.m_glfw_window);
    std::swap(m_resize_func, move.m_resize_func);

    glfwSetWindowUserPointer(GetGlfwWindow(), this);
    glfwSetWindowUserPointer(move.GetGlfwWindow(), &move);
    
    return *this;
}
    
void Window::GlfwWindowDeleter::operator()(GLFWwindow* ptr) const noexcept {
    glfwDestroyWindow(ptr);
}

void Window::OnResize(std::function<OnWindowResizeFunc> func) {
    // window_m_resize_funcs.insert_or_assign(GetGlfwWindow(), std::move(func));
    m_resize_func = std::move(func);
}

void Window::MakeCurrent() const {
    glfwMakeContextCurrent(GetGlfwWindow());
}

void Window::SwapBuffers() { // NOLINT
    glfwSwapBuffers(GetGlfwWindow());
}

void Window::PollEvents() { // NOLINT
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

Window& Window::GetWindowFromGlfw(GLFWwindow* m_glfw_window) {
    return *reinterpret_cast<Window*>(glfwGetWindowUserPointer(m_glfw_window));
}

void Window::OnWindowResizeImpl(GLFWwindow* m_glfw_window, int width, int height) {
    // Don't propogate size updates when area is 0
    if (width == 0 || height == 0)
        return;
    
    glfwMakeContextCurrent(m_glfw_window);
    glViewport(0, 0, width, height);
    
    GetWindowFromGlfw(m_glfw_window).m_resize_func({width, height});
}

SubWindow::SubWindow(dxtl::cstring_view title, glm::ivec2 window_size, const Window& parent)
    : Window(title, window_size, &parent) { }

SubWindow::SubWindow(dxtl::cstring_view title, Fullscreen, const Window& parent)
    : Window(title, Fullscreen{}, &parent) { }

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
