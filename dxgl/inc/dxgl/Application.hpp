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
            void operator()(GLFWwindow* ptr) const;
        };
        std::unique_ptr<GLFWwindow, GlfwWindowDeleter> glfw_window;

        std::function<OnWindowResizeFunc> resize_func{};
    public:
        struct Fullscreen { };

        Window(dxtl::cstring_view title, glm::ivec2 window_size, const Window* share = nullptr);
        Window(dxtl::cstring_view title, Fullscreen, const Window* share = nullptr);

        void MakeCurrent() const;
        void SwapBuffers();
        void PollEvents();

        void OnResize(std::function<OnWindowResizeFunc> func);
        
        glm::ivec2 GetSize() const;
        glm::dvec2 GetMousePos() const;
        glm::vec2 GetScale() const;
        bool ShouldClose() const;

        GLFWwindow* GetGlfwWindow() const { return glfw_window.get(); }

    private:
        static void OnWindowResizeImpl(GLFWwindow* window, int w, int h);
    };

    namespace Application {
        void Init();
        void Terminate();

        double GetTime();
    };
}