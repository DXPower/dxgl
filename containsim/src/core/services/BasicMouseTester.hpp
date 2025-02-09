#include <services/interfaces/IMouseTester.hpp>
#include <dxgl/Application.hpp>
#include <RmlUi/Core/Context.h>

namespace services {
    struct BasicMouseTester : services::IMouseTester {
        const dxgl::Window* window{};
        Rml::Context* gui{};

        BasicMouseTester(const dxgl::Window& window, Rml::Context& gui) : window(&window), gui(&gui) {}

        services::InputLayer TestMouse(glm::dvec2 pos) const override {
            if (pos.x < 0 || pos.x >= window->GetSize().x)
                return services::InputLayer::Offscreen;

            if (pos.y < 0 || pos.y >= window->GetSize().y)
                return services::InputLayer::Offscreen;

            // if (gui->IsMouseInteracting())
            //     return services::InputLayer::Ui;

            return services::InputLayer::Game;             
        }
    };
}