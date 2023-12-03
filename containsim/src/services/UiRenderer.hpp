#pragma once

#include <common/Action.hpp>

#include <dxgl/Application.hpp>
#include <dxtl/cstring_view.hpp>

#include <memory>
#include <string_view>

class DrawQueues;

namespace services {
    class UiRenderer {
        class Pimpl;
        std::unique_ptr<Pimpl> m_pimpl{};

    public:
        UiRenderer(const dxgl::Window& main_window, const dxgl::Window& inspector_window);
        ~UiRenderer();

        void Update();
        void Render(DrawQueues& draw_queues) const;
        void RenderDebug(DrawQueues& draw_queues) const;

        void InputAction(const Action& action);
        void InputActionDebug(const Action& action);

        void LoadHtml(std::string_view path);
        void LoadCss(std::string_view path); 
        void LoadUrl(dxtl::cstring_view path);

        void Resize(glm::ivec2 size);
        void ResizeInspector(glm::ivec2 size);
    };
}