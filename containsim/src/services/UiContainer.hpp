#pragma once

#include <services/UiView.hpp>

namespace services {
    class UiContainer {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* ptr) const;
        };
        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        UiContainer(const dxgl::Window& main_window, const dxgl::Window& inspector_window);
        ~UiContainer();

        void Update();
        void Render();

        UiView& GetMainView();
        const UiView& GetMainView() const;

        UiView& GetInspectorView();
        const UiView& GetInspectorView() const;
    };
}