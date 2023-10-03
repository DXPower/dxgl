#pragma once

#include <functional>

#include <dxtl/cstring_view.hpp>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace dxgl {
    using OnWindowResizeFunc = void(int w, int h);

    namespace Application {
        struct Fullscreen { };

        void Init(dxtl::cstring_view title, int screenW, int screenH);
        void Init(dxtl::cstring_view title, Fullscreen);

        void SwapBuffers();
        void PollEvents();
        void Destroy();

        void OnWindowResize(std::function<OnWindowResizeFunc> func);

        double GetTime();
        glm::vec<2, int> GetWindowSize();
        bool ShouldQuit();

        GLFWwindow* GetWindow();
    };
}