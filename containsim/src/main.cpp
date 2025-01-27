#include <common/GlobalConfig.hpp>
#include <dxgl/Texture.hpp>
#include <exception>
#include <iostream>
#include <charconv>

#include <flecs.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <dxgl/Application.hpp>
#include <dxgl/Screenbuffer.hpp>
#include <dxgl/Ubo.hpp>

#include <common/DrawQueues.hpp>
#include <components/RenderData.hpp>
#include <components/Sprite.hpp>
#include <components/Transform.hpp>
#include <systems/SpriteRenderer.hpp>
#include <services/ActionRouter.hpp>
#include <services/BuildInput.hpp>
#include <services/InputState.hpp>
#include <services/TileGrid.hpp>
#include <services/TileGridRenderer.hpp>
#include <services/InputHandler.hpp>
#include <services/BasicMouseTester.hpp>
#include <services/Logging.hpp>
#include <services/WindowService.hpp>
#include <services/Camera.hpp>
#include <RmlUi/Core.h>
#include <RmlUi_Backends/RmlUi_Backend.h>
#include <RmlUi_Backends/RmlUi_Renderer_GL3.h>
#include <RmlUi_Backends/RmlUi_Platform_GLFW.h>
#include <services/UiActionReceiver.hpp>
#include <common/GlobalConfig.hpp>

#include <spdlog/spdlog.h>
#include <kangaru/kangaru.hpp>

using namespace dxgl;

struct InputResults {
    glm::vec2 camera_movement{};
};

static bool cycle_tiles = false;

struct GlobalActions : ActionConsumer {
    void Consume(Action&& action) override {
        const KeyPress* press = std::get_if<KeyPress>(&action.data);
        if (press != nullptr && press->IsDownKey(GLFW_KEY_E)) {
            cycle_tiles = true;
        }
    }
};


static InputResults ProcessInput(GLFWwindow* window) {
    InputResults result{};

    auto IsKeyPressed = [window](int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    };

    // if (IsKeyPressed(GLFW_KEY_ESCAPE))
    //     glfwSetWindowShouldClose(window, 1);
    
    if (IsKeyPressed(GLFW_KEY_W))
        result.camera_movement.y -= 1;
    if (IsKeyPressed(GLFW_KEY_S))
        result.camera_movement.y += 1;
    if (IsKeyPressed(GLFW_KEY_A))
        result.camera_movement.x -= 1;
    if (IsKeyPressed(GLFW_KEY_D))
        result.camera_movement.x += 1;

    if (result.camera_movement != glm::vec2(0, 0))
        result.camera_movement = glm::normalize(result.camera_movement);

    return result;
}


static void Clear() {
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // NOLINT
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


int main() {
    namespace logging = services::logging;
    logging::SetCommonSink(logging::CreateConsoleSink());

    constexpr glm::ivec2 initial_screen_size = { 1000, 800 };

    try {
        Application::Init();

        Window main_window(dxtl::cstring_view("Containment Simulator"), initial_screen_size);
        main_window.MakeCurrent();
        
        if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, initial_screen_size.x, initial_screen_size.y);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);

        RenderInterface_GL3 rml_renderer{};
        SystemInterface_GLFW rml_system{};

        Rml::SetRenderInterface(&rml_renderer);
        Rml::SetSystemInterface(&rml_system);

        rml_renderer.SetViewport(initial_screen_size.x, initial_screen_size.y);

        Rml::Initialise();

        services::UiActionReceiver ui_actions{main_window};
        auto* rml_context = Rml::CreateContext(
            "main",
            Rml::Vector2i(initial_screen_size.x, initial_screen_size.y),
            nullptr,
            &ui_actions
        ); 

        ui_actions.SetContext(*rml_context);


        if (rml_context == nullptr) {
            throw std::runtime_error("Failed to create RmlUi context");
        }

        if (!Rml::LoadFontFace("res/fonts/LatoLatin-Regular.ttf", true)) {
            throw std::runtime_error("Failed to load RmlUi font face");
        }

        auto document = rml_context->LoadDocument("res/ui/game/test.rml");

        if (document == nullptr) {
            throw std::runtime_error("Failed to load RmlUi document");
        }

        document->Show();

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.Resize(initial_screen_size);

        UboBindingManager ubos{};
        services::Camera camera(ubos);
        camera.UpdateViewportSize(initial_screen_size);

        flecs::world world{};

        using namespace components;
        
        DrawQueues draw_queues{};
        systems::SpriteRenderer sprite_renderer(ubos, draw_queues);


        std::function<void(
            const components::SpriteRenderer&,
            const components::Transform&,
            const components::RenderData&,
            const components::Sprite&
        )> pre_store = std::bind_front(&systems::SpriteRenderer::PreStore, &sprite_renderer);

        world.system<
            components::SpriteRenderer,
            components::Transform,
            components::RenderData,
            components::Sprite>(
            "SpriteRenderer"
        )
            .kind(flecs::PreStore)
            .each(pre_store);

        GlobalConfig global_config{
            .map_size = { 20, 10 }, // NOLINT
            .tile_size = {100, 100} // NOLINT
        };

        services::TileGrid tile_grid(global_config);
        services::TileGridRenderer tile_grid_renderer(tile_grid, ubos);

        auto logger = services::logging::CreateLogger("main");
        logger.info("Hello, world!");
        logger.set_level(spdlog::level::debug);

        auto SetTiles = [&, start = 0]() mutable {
            logger.debug("start: {}", start);

            for (int x = 0; x < global_config.map_size.x; x++) {
                for (int y = 0; y < global_config.map_size.y; y++) {
                    TileData data{};
                    data.type = static_cast<TileType>(start);
                    tile_grid.SetTile({x, y}, data);
                    
                    start = (int) ((start + 1) % magic_enum::enum_count<TileType>());
                }
            }
        };

        SetTiles();

        main_window.OnResize([&](glm::ivec2 size) {
            main_screen_buffer.Resize(size);
            camera.UpdateViewportSize(size);
            rml_context->SetDimensions(Rml::Vector2i(size.x, size.y));
            rml_renderer.SetViewport(size.x, size.y);
        });

        float last_time{};
        constexpr float camera_speed = 350.f;

        services::BasicMouseTester mouse_tester{main_window, *rml_context};
        services::ActionRouter input_router{mouse_tester};

        services::InputHandler game_input{main_window};

        chain::Connect(game_input.actions_out, ui_actions);
        chain::Connect(ui_actions.uncaptured_actions, input_router);

        services::BuildInput build_input{};
        services::InputState input_state{build_input};

        chain::Connect(input_router.game_action_receiver, input_state);
        chain::Connect(input_router.offscreen_action_receiver, input_state);
        chain::ConnectToNull(input_router.ui_action_receiver);

        GlobalActions global_actions{};

        chain::Connect(input_state.build_input_cmds, build_input);
        chain::Connect(input_state.build_actions, build_input);
        chain::Connect(build_input.uncaptured_actions, global_actions);
        chain::Connect(input_state.idle_actions, global_actions);
        
        // ui_container.GetMainView()
        //     .RegisterCallback(
        //         "EnterBuildMode",
        //         services::MakeUiCallback([&]() {
        //             logger.info("EnterBuildMode");
        //             input_state.EnterBuildMode();
        //         })
        //     );

        // ui_container.GetMainView()
        //     .RegisterCallback(
        //         "ExitBuildMode",
        //         services::MakeUiCallback([&]() {
        //             logger.info("ExitBuildMode");
        //             input_state.ExitMode();
        //         })
        //     );

        // ui_container.GetMainView()
        //     .RegisterCallback(
        //         "SelectTileToPlace",
        //         services::MakeUiCallback<UiTypes::String>([&](std::string str) {
        //             std::underlying_type_t<TileType> type_i{};
        //             std::from_chars(str.data(), str.data() + str.size(), type_i);

        //             logger.info("SelectTileToPlace: {}", type_i);
        //             auto type = static_cast<TileType>(type_i);

        //             build_input.SelectTileToPlace(type);
        //         })
        //     );

        // ui_container.GetMainView()
        //     .RegisterCallback(
        //         "EnterDeleteMode",
        //         services::MakeUiCallback<>([&] {
        //             logger.info("EnterDeleteMode");
        //             build_input.EnterDeleteMode();
        //         })
        //     );

        // ui_container.GetMainView()
        //     .RegisterCallback(
        //         "ExitDeleteMode",
        //         services::MakeUiCallback<>([&] {
        //             logger.info("ExitDeleteMode");
        //             build_input.ExitMode();
        //         })
        //     );

        while (!main_window.ShouldClose()) {
            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);
            draw_queues = {};

            auto input [[maybe_unused]] = ProcessInput(main_window.GetGlfwWindow());
            float current_time = (float) Application::GetTime();
            float delta_time = current_time - last_time;
            last_time = current_time;

            camera.MoveBy(input.camera_movement * camera_speed * delta_time);

            world.progress();

            if (cycle_tiles) {
                SetTiles();
                cycle_tiles = false;
            }

            tile_grid_renderer.Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_screen_buffer.Render();

            dxgl::Screenbuffer::Unuse();

            rml_context->Update();
            rml_renderer.BeginFrame();
            rml_context->Render();
            rml_renderer.EndFrame();

            main_window.SwapBuffers();
            main_window.PollEvents();
        }

        Rml::Shutdown();
        Application::Terminate();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}
