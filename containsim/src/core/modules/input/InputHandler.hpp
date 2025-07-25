#pragma once

#include <common/Action.hpp>
#include <common/ActionChain.hpp>

#include <memory>

#include <dxgl/Application.hpp>

namespace input {
    class InputHandler {
    public:
        ActionProducer actions_out{};

    private:
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* ptr) const;
        };
        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        InputHandler(const dxgl::Window& window);
        ~InputHandler();
    };
}