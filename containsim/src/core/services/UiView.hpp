#pragma once

#include <common/Action.hpp>
#include <common/ActionChain.hpp>
#include <services/UiCallback.hpp>
#include <services/interfaces/IMouseTester.hpp>
#include <services/interfaces/IActionReceiver.hpp>

#include <dxgl/Application.hpp>
#include <dxtl/cstring_view.hpp>

#include <concepts>
#include <memory>
#include <string_view>

class DrawQueues;

namespace ultralight {
    template<typename T>
    class RefPtr;

    class View;
}

namespace services {
    class UiView : public IMouseTester, public ActionConsumer {
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

        void Consume(Action&& action) override;
        InputLayer TestMouse(glm::dvec2 pos) const override;

        void LoadHtml(std::string_view path);
        void LoadUrl(dxtl::cstring_view path);

        void Resize(glm::ivec2 size);

        void RegisterCallback(dxtl::cstring_view js_name, UiCallback&& callback);
        void UnregisterCallback(dxtl::cstring_view js_name);

        template<typename... T>
        requires (std::constructible_from<UiValue, T> && ...)
        UiValue CallFunction(dxtl::cstring_view js_name,  T&&... args) {
            std::vector<UiValue> ui_args{};
            (ui_args.emplace_back(std::forward<T>(args)), ...);
            
            return CallFunctionImpl(js_name, ui_args);
        }

        ultralight::RefPtr<ultralight::View> GetView();

    private:
        UiValue CallFunctionImpl(dxtl::cstring_view js_name, std::span<UiValue> args);
    };
}