#include <common/GlobalConfig.hpp>
#include <dxgl/Texture.hpp>
#include <exception>
#include <iostream>
#include <charconv>

#include <flecs.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <dxgl/Application.hpp>
#include <dxgl/Screenbuffer.hpp>

#include <common/DrawQueuesKgr.hpp>
#include <common/DxglKgr.hpp>
#include <components/RenderData.hpp>
#include <components/Sprite.hpp>
#include <components/Transform.hpp>
#include <systems/SpriteRendererKgr.hpp>
#include <services/ActionRouterKgr.hpp>
#include <services/BuildServicesKgr.hpp>
#include <services/InputStateKgr.hpp>
#include <services/TileGridKgr.hpp>
#include <services/TileGridRendererKgr.hpp>
#include <services/InputHandlerKgr.hpp>
#include <services/UiViewKgr.hpp>
#include <services/UiContainer.hpp>
#include <services/ActionRouter.hpp>
#include <services/Logging.hpp>
#include <services/WindowService.hpp>
#include <services/CameraKgr.hpp>

#include <common/GlobalConfigKgr.hpp>

#include <spdlog/spdlog.h>
#include <kangaru/kangaru.hpp>

using namespace dxgl;

static bool is_ui_loaded = true;
static bool toggle_ui{};

struct InputResults {
    glm::vec2 camera_movement{};
};

struct GlobalActions : ActionConsumer {
    void Consume(Action&& action) override {
        const KeyPress* press = std::get_if<KeyPress>(&action.data);
        if (press != nullptr && press->IsDownKey(GLFW_KEY_P)) {
            toggle_ui ^= 1;
        }
    }
};


InputResults ProcessInput(GLFWwindow* window) {
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

bool cycle_tiles = false;

void Clear() {
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // NOLINT
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


int main() {
    namespace logging = services::logging;
    logging::SetCommonSink(logging::CreateConsoleSink());

    constexpr glm::ivec2 initial_screen_size = { 1000, 800 };
    constexpr glm::ivec2 initial_debug_screen_size = { 800, 800 };
    try {
        Application::Init();

        kgr::container main_di{};
        kgr::container debug_di{};

        main_di.emplace<services::WindowService>(dxtl::cstring_view("Containment Simulator"), initial_screen_size);
        auto& main_window = main_di.service<services::WindowService>();
        main_window.MakeCurrent();
        
        if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, initial_screen_size.x, initial_screen_size.y);

        debug_di.emplace<services::SubWindowService>("Containment Simulator Debug", initial_debug_screen_size, main_window);
        auto& debug_window = debug_di.service<services::SubWindowService>();
        debug_window.MakeCurrent();

        glViewport(0, 0, initial_debug_screen_size.x, initial_debug_screen_size.y);

        main_window.MakeCurrent();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.Resize(initial_screen_size);

        auto& camera = main_di.service<kgr::service_for<services::Camera>>();
        camera.UpdateViewportSize(initial_screen_size);

        flecs::world world{};

        using namespace components;
        
        auto& draw_queues = main_di.service<DrawQueuesService>();
        auto& sprite_renderer = main_di.service<kgr::service_for<systems::SpriteRenderer>>();        

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

        main_di.emplace<GlobalConfigService>(GlobalConfig{
            .map_size = { 20, 10 }, // NOLINT
            .tile_size = {100, 100} // NOLINT
        });

        auto& tile_grid = main_di.service<kgr::service_for<services::TileGrid>>();
        auto& tile_grid_renderer = main_di.service<kgr::service_for<services::TileGridRenderer>>();

        auto logger = services::logging::CreateLogger("main");
        logger.info("Hello, world!");
        logger.set_level(spdlog::level::debug);

        auto SetTiles = [&, start = 0]() mutable {
            logger.debug("start: {}", start);

            auto& global_config = main_di.service<GlobalConfigService>();
            
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

        services::UiContainer ui_container(main_window, debug_window);
        main_di.emplace<services::UiViewService>(ui_container.GetMainView());
        ui_container.GetMainView().LoadUrl("file:///game/ingame.html");
        
        main_window.OnResize([&](glm::ivec2 size) {
            main_screen_buffer.Resize(size);
            camera.UpdateViewportSize(size);
            ui_container.GetMainView().Resize(size);
        });

        debug_window.OnResize([&](glm::ivec2 size) {
            ui_container.GetInspectorView().Resize(size);
        });

        float last_time{};
        constexpr float camera_speed = 350.f;

        auto& debug_input = debug_di.service<kgr::service_for<services::InputHandler>>();
        chain::Connect(debug_input.actions_out, ui_container.GetInspectorView());

        auto input_router = main_di.service<services::ActionRouterService>();

        auto& game_input = main_di.service<kgr::service_for<services::InputHandler>>();
        chain::Connect(game_input.actions_out, input_router);

        auto& build_input = main_di.service<kgr::service_for<services::BuildInput>>();
        auto& input_state = main_di.service<kgr::service_for<services::InputState>>();

        chain::Connect(input_router.game_action_receiver, input_state);
        chain::Connect(input_router.offscreen_action_receiver, input_state);
        chain::Connect(input_router.ui_action_receiver, ui_container.GetMainView());

        GlobalActions global_actions{};

        chain::Connect(input_state.build_input_cmds, build_input);
        chain::Connect(input_state.build_actions, build_input);
        chain::Connect(build_input.uncaptured_actions, global_actions);
        chain::Connect(input_state.idle_actions, global_actions);
        
        ui_container.GetMainView()
            .RegisterCallback(
                "EnterBuildMode",
                services::MakeUiCallback([&]() {
                    logger.info("EnterBuildMode");
                    input_state.EnterBuildMode();
                })
            );

        ui_container.GetMainView()
            .RegisterCallback(
                "ExitBuildMode",
                services::MakeUiCallback([&]() {
                    logger.info("ExitBuildMode");
                    input_state.ExitMode();
                })
            );

        ui_container.GetMainView()
            .RegisterCallback(
                "SelectTileToPlace",
                services::MakeUiCallback<UiTypes::String>([&](std::string str) {
                    std::underlying_type_t<TileType> type_i{};
                    std::from_chars(str.data(), str.data() + str.size(), type_i);

                    logger.info("SelectTileToPlace: {}", type_i);
                    auto type = static_cast<TileType>(type_i);

                    build_input.SelectTileToPlace(type);
                })
            );

        ui_container.GetMainView()
            .RegisterCallback(
                "EnterDeleteMode",
                services::MakeUiCallback<>([&] {
                    logger.info("EnterDeleteMode");
                    build_input.EnterDeleteMode();
                })
            );

        ui_container.GetMainView()
            .RegisterCallback(
                "ExitDeleteMode",
                services::MakeUiCallback<>([&] {
                    logger.info("ExitDeleteMode");
                    build_input.ExitMode();
                })
            );

        while (!main_window.ShouldClose() && !debug_window.ShouldClose()) {
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

            // Update and render UI
            if (toggle_ui) {
                if (is_ui_loaded) {
                    logger.info("Unloading UI...");
                    ui_container.GetMainView().LoadUrl("https://www.cloudflare.com/cdn-cgi/trace");
                } else {
                    logger.info("Loading UI...");
                    ui_container.GetMainView().LoadUrl("file:///game/ingame.html");
                }

                is_ui_loaded ^= true;
                toggle_ui = false;
            }

            ui_container.Update();
            ui_container.Render();

            dxgl::Screenbuffer::Unuse();

            ui_container.GetMainView().Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_window.SwapBuffers();
            main_window.PollEvents();

            debug_window.MakeCurrent();
            Clear();

            ui_container.GetInspectorView().Render(draw_queues);

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            debug_window.SwapBuffers();
            debug_window.PollEvents();

            main_window.MakeCurrent();
        }

        Application::Terminate();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
