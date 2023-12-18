#pragma once

#include <common/Action.hpp>

#include <functional>
#include <memory>

#include <dxgl/Application.hpp>

namespace services {
    class InputHandler {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* ptr) const;
        };
        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        InputHandler(const dxgl::Window& window);
        ~InputHandler();

        void OnAction(std::function<void(Action&& action)> func);
    };
}