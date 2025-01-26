#include <services/interfaces/IMouseTester.hpp>
#include <dxgl/Application.hpp>

namespace services {
    struct BasicMouseTester : services::IMouseTester {
        const dxgl::Window* window{};

        BasicMouseTester(const dxgl::Window& window) : window(&window) {}

        services::InputLayer TestMouse(glm::dvec2 pos) const override {
            if (pos.x < 0 || pos.x > window->GetSize().x)
                return services::InputLayer::Offscreen;

            if (pos.y < 0 || pos.y > window->GetSize().y)
                return services::InputLayer::Offscreen;

            return services::InputLayer::Game;             
        }
    };
}

#include <services/interfaces/IMouseTesterKgr.hpp>

namespace services {
    struct BasicMouseTesterService : kgr::autowire_single_service<BasicMouseTester>, kgr::overrides<MouseTesterService> { };
    // auto service_map(const BasicMouseTesterService&) -> BasicMouseTesterService;
}