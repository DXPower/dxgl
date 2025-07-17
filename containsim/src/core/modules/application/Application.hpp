#pragma once

#include <modules/application/UiEnv.hpp>
#include <modules/application/EventManager.hpp>
#include <modules/application/WindowComponents.hpp>

#include <flecs.h>

namespace application {
    // This module is responsible for initializing the window and general environment.
    // This includes OpenGL/GLAD, GLFW, and the UI.
    class Application {
    public:
        Application(flecs::world& world);
    };
}