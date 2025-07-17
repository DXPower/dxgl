#pragma once

#include <modules/input/IMouseTester.hpp>
#include <dxgl/Application.hpp>
#include <RmlUi/Core/Context.h>

namespace input {
    class BasicMouseTester : IMouseTester {
        const dxgl::Window* m_window{};

    public:
        BasicMouseTester(const dxgl::Window& window) : m_window(&window) {}

        InputLayer TestMouse(glm::dvec2 pos) const override {
            if (pos.x < 0 || pos.x >= m_window->GetSize().x)
                return InputLayer::Offscreen;

            if (pos.y < 0 || pos.y >= m_window->GetSize().y)
                return InputLayer::Offscreen;

            return InputLayer::Game;             
        }
    };
}