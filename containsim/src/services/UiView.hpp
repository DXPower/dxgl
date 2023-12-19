#pragma once

#include <common/Action.hpp>
#include <services/UiCallback.hpp>
#include <services/interfaces/IMouseTester.hpp>
#include <services/interfaces/IActionReceiver.hpp>

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
    class UiView : public IMouseTester, public IActionReceiver {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* ptr) const;
        };
        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        UiView(const dxgl::Window& window, const ultralight::RefPtr<ultralight::View>& view);
        ~UiView();

        void Update();
        void Render(DrawQueues& draw_queues) const;

        void PushAction(Action&& action) override;
        InputLayer TestMouse(glm::dvec2 pos) const override;

        void LoadHtml(std::string_view path);
        void LoadUrl(dxtl::cstring_view path);

        void Resize(glm::ivec2 size);

        void RegisterCallback(std::string_view js_name, UiCallback&& callback);
        void UnregisterCallback(std::string_view js_name);

        ultralight::RefPtr<ultralight::View> GetView();
    };
}