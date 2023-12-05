#pragma once

#include <common/Action.hpp>

#include <dxgl/Application.hpp>
#include <dxtl/cstring_view.hpp>

#include <memory>
#include <string_view>

class DrawQueues;

namespace ultralight {
    template<typename T>
    class RefPtr;

    class View;
}

namespace services {
    class UiView {
        class Pimpl;
        std::unique_ptr<Pimpl> m_pimpl{};

    public:
        UiView(const dxgl::Window& window, const ultralight::RefPtr<ultralight::View>& view);
        ~UiView();

        void Update();
        void Render(DrawQueues& draw_queues) const;

        void InputAction(const Action& action);

        void LoadHtml(std::string_view path);
        void LoadUrl(dxtl::cstring_view path);

        void Resize(glm::ivec2 size);

        ultralight::RefPtr<ultralight::View> GetView();
    };
}