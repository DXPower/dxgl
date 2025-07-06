#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <modules/application/Application.hpp>

using namespace application;

namespace {

void InitGlad(glm::ivec2 screen_size) {
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glViewport(0, 0, screen_size.x, screen_size.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
}

}

Application::Application(flecs::world& world) {
    world.component<EventManager>().add(flecs::Sparse);
    world.add<EventManager>();

    auto& event_manager = world.get_mut<EventManager>();

    world.component<dxgl::Window>().add(flecs::Sparse);
    world.component<MainWindow>();
    world.component<WindowSize>();

    dxgl::Application::Init();

    // Make the main window entity
    constexpr glm::ivec2 initial_screen_size = { 1000, 800 };

    auto main_window_e = world.entity()
        .emplace<dxgl::Window>(dxtl::cstring_view("Containment Simulator"), initial_screen_size)
        .set<WindowSize>({initial_screen_size})
        .add<MainWindow>();

    auto& main_window = main_window_e.get_mut<dxgl::Window>();
    main_window.MakeCurrent();

    main_window.OnResize([main_window_e](glm::ivec2 new_size) {
        main_window_e.set<WindowSize>({new_size});
    });

    InitGlad(initial_screen_size);



    world.component<UiEnv>().add(flecs::Sparse);
    world.emplace<UiEnv>(main_window_e);
}