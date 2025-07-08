#pragma once

#include <memory>
#include <functional>

#include <dxtl/cstring_view.hpp>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace dxgl {
    using OnWindowResizeFunc = void(glm::ivec2);

    class Window {
        struct GlfwWindowDeleter {
            void operator()(GLFWwindow* ptr) const noexcept;
        };
        std::unique_ptr<GLFWwindow, GlfwWindowDeleter> m_glfw_window;

        std::function<OnWindowResizeFunc> m_resize_func{};
    public:
        struct Fullscreen { };
        struct BorderlessWindow { };

        Window(dxtl::cstring_view title, glm::ivec2 window_size);
        Window(dxtl::cstring_view title, Fullscreen);
        Window(dxtl::cstring_view title, BorderlessWindow);
    protected:
        Window(dxtl::cstring_view title, glm::ivec2 window_size, const Window* share);
        Window(dxtl::cstring_view title, Fullscreen, const Window* share);
        Window(dxtl::cstring_view title, BorderlessWindow, const Window* share);

    public:
        Window(const Window& copy) = delete;
        Window(Window&& move) noexcept;
        virtual ~Window() noexcept = default;

        Window& operator=(Window&& move) noexcept;

        void MakeCurrent() const;
        void SwapBuffers();
        void PollEvents();

        void OnResize(std::function<OnWindowResizeFunc> func);
        
        glm::ivec2 GetSize() const;
        glm::dvec2 GetMousePos() const;
        glm::vec2 GetScale() const;
        bool ShouldClose() const;

        GLFWwindow* GetGlfwWindow() const { return m_glfw_window.get(); }
        static Window& GetWindowFromGlfw(GLFWwindow* glfw_window);
    private:
        static void OnWindowResizeImpl(GLFWwindow* window, int w, int h);
    };

    class SubWindow : public Window {
    public:
        SubWindow(dxtl::cstring_view title, glm::ivec2 window_size, const Window& parent);
        SubWindow(dxtl::cstring_view title, Fullscreen, const Window& parent);
    };

    namespace Application {
        void Init();
        void Terminate();

        double GetTime();
    };
}