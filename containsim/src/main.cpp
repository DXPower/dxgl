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
#include <services/BuildManager.hpp>
#include <services/RoomManager.hpp>
#include <services/InputState.hpp>
#include <services/TileGrid.hpp>
#include <services/TileGridRenderer.hpp>
#include <services/RoomRenderer.hpp>
#include <services/InputHandler.hpp>
#include <services/BasicMouseTester.hpp>
#include <services/Logging.hpp>
#include <services/Camera.hpp>
#include <systems/TilePrefabs.hpp>
#include <RmlUi/Core.h>
#include <RmlUi_Backends/RmlUi_Backend.h>
#include <RmlUi_Backends/RmlUi_Renderer_GL3.h>
#include <RmlUi_Backends/RmlUi_Platform_GLFW.h>
#include <RmlUi/Debugger.h>
#include <services/UiActionReceiver.hpp>
#include <services/ui/Panel.hpp>
#include <services/ui/BuildPanel.hpp>
#include <services/ui/RoomPanel.hpp>
#include <services/ui/RmlEventManager.hpp>
#include <services/ui/InputStateBinding.hpp>
#include <services/ui/TilesBinding.hpp>
#include <services/EventManager.hpp>
#include <systems/CircleMover.hpp>
#include <systems/Collision.hpp>
#include <components/Collider.hpp>
#include <components/Pathing.hpp>
#include <components/Actor.hpp>
#include <components/Mobility.hpp>
#include <systems/Pathfinder.hpp>
#include <systems/PathMover.hpp>
#include <common/DebugDraws.hpp>
#include <spdlog/spdlog.h>
#include <kangaru/kangaru.hpp>

using namespace dxgl;

struct InputResults {
    glm::vec2 camera_movement{};
};

static bool cycle_tiles = false;
static bool toggle_debugger = false;
static std::optional<glm::vec2> target_pos{};

struct GlobalActions : ActionConsumer {
    void Consume(Action&& action) override {
        const KeyPress* press = std::get_if<KeyPress>(&action.data);
        if (press != nullptr && press->IsDownKey(GLFW_KEY_E)) {
            cycle_tiles = true;
        }

        if (press != nullptr && press->IsDownKey(GLFW_KEY_F8)) {
            toggle_debugger = true;
        }

        const MouseClick* click = std::get_if<MouseClick>(&action.data);
        if (click != nullptr && click->button == 0 && click->dir == ButtonDir::Up) {
            target_pos = click->pos;
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

#define CATCH_EXCEPTIONS 1

int main() {
    namespace logging = services::logging;
    logging::SetCommonSink(logging::CreateConsoleSink());

    constexpr glm::ivec2 initial_screen_size = { 1000, 800 };

#if CATCH_EXCEPTIONS == 1
    try {
#endif        
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

        if (rml_context == nullptr) {
            throw std::runtime_error("Failed to create RmlUi context");
        }

        rml_context->SetDensityIndependentPixelRatio(main_window.GetScale().x);

        ui_actions.SetContext(*rml_context);
        Rml::Debugger::Initialise(rml_context);

        if (!Rml::LoadFontFace("res/fonts/LatoLatin-Regular.ttf", true)) {
            throw std::runtime_error("Failed to load RmlUi font face");
        }

        services::EventManager event_manager{};
        services::ui::InputStateBinding input_state_binding{
            services::InputStates::IdleMode,
            *rml_context,
            event_manager
        };
        services::ui::TilesBinding tiles_binding{*rml_context};

        services::ui::RmlEventManager ui_event_manager{event_manager};
        Rml::Factory::RegisterEventListenerInstancer(&ui_event_manager);

        auto document = rml_context->LoadDocument("res/ui/game/test.rml");

        if (document == nullptr) {
            throw std::runtime_error("Failed to load RmlUi document");
        }

        document->Show();


        services::ui::BuildPanel build_panel{event_manager, *document};
        services::ui::RoomPanel room_panel{event_manager, *document};

        Screenbuffer main_screen_buffer{};
        main_screen_buffer.Resize(initial_screen_size);

        UboBindingManager ubos{};
        services::Camera camera(ubos);
        camera.UpdateViewportSize(initial_screen_size);

        flecs::world world{};

        using namespace components;
        
        DrawQueues draw_queues{};
        DebugDraws::Init(ubos, draw_queues);

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

        services::InitTilePrefabs(world, global_config);
        services::TileGrid tile_grid(global_config, world);
        services::TileGridRenderer tile_grid_renderer(tile_grid, ubos);

        auto logger = services::logging::CreateLogger("main");
        logger.info("Hello, world!");
        logger.set_level(spdlog::level::debug);

        auto SetTiles = [&, start = 0]() mutable {
            constexpr static std::array types = {
                TileType::Grass,
                TileType::Dirt,
                TileType::Tile
            };

            for (int x = 0; x < global_config.map_size.x; x++) {
                for (int y = 0; y < global_config.map_size.y; y++) {
                    TileData data{};
                    data.type = types[(start + x + y) % types.size()]; // NOLINT
                    tile_grid.SetTile({x, y}, TileLayer::Ground, data);
                }
            }
            
            start++;
        };

        SetTiles();

        main_window.OnResize([&](glm::ivec2 size) {
            main_screen_buffer.Resize(size);
            camera.UpdateViewportSize(size);
            rml_context->SetDimensions(Rml::Vector2i(size.x, size.y));
            rml_context->SetDensityIndependentPixelRatio(main_window.GetScale().x);
            rml_renderer.SetViewport(size.x, size.y);
        });

        double last_time{};
        constexpr float camera_speed = 350.f;

        services::BasicMouseTester mouse_tester{main_window, *rml_context};
        services::ActionRouter input_router{mouse_tester};

        services::InputHandler game_input{main_window};

        chain::Connect(game_input.actions_out, ui_actions);
        chain::Connect(ui_actions.uncaptured_actions, input_router);

        services::BuildInput build_input{event_manager, camera, tile_grid};
        services::RoomInput room_input{event_manager, camera, tile_grid};
        services::InputState input_state{event_manager, build_input, room_input};

        chain::Connect(input_router.game_action_receiver, input_state);
        chain::Connect(input_router.offscreen_action_receiver, input_state);
        chain::ConnectToNull(input_router.ui_action_receiver);

        services::BuildManager build_manager{tile_grid};
        chain::Connect(build_input.build_commands, build_manager);

        services::RoomManager room_manager{tile_grid};
        services::RoomRenderer room_renderer(room_manager, ubos);

        chain::Connect(room_input.room_commands, room_manager);

        GlobalActions global_actions{};

        // chain::Connect(input_state.build_input_cmds, build_input);
        chain::Connect(input_state.idle_actions, global_actions);
        chain::Connect(input_state.build_actions, build_input);
        chain::Connect(build_input.uncaptured_actions, global_actions);
        chain::Connect(input_state.room_actions, room_input);
        chain::Connect(room_input.uncaptured_actions, global_actions);

        systems::Pathfinder pathfinder{tile_grid};
        pathfinder.PreUpdate(world);
        systems::PathMover(world);
        systems::Collision(world);

        struct PlayerTest { };

        // Populate world actors
        dxgl::Texture gardner_sheet = dxgl::LoadTextureFromFile("res/img/gardner.png");
        auto test_actor = world.entity();
        test_actor.set<components::Actor>(components::Actor{.id = 1});
        test_actor.add<PlayerTest>();
        test_actor.set<components::PathMover>(components::PathMover{});
        test_actor.set(components::Transform{
            .position = {400, 400},
            .size = {95, 95},
        });
        test_actor.set(components::Sprite{
            .spritesheet = gardner_sheet,
            .cutout = {
                .position = {0, 0},
                .size = {64, 64}
            }
        });
        test_actor.set(components::RenderData{
            .layer = RenderLayer::Objects
        });
        test_actor.add<components::SpriteRenderer>();
        test_actor.set<components::Mobility>(components::Mobility{.speed = 350.f});
        test_actor.set(components::Collider{
            .is_listening = true
        });
        test_actor.set(components::SquareCollider{
            .relative_size = {.9f, .95f}
        });
        
        for (int i = 0; i < 2; i++) {
            auto other_actor = world.entity();
            other_actor.set<components::Actor>(components::Actor{.id = 2 + (unsigned int)i});
            other_actor.set(components::Transform{
                .position = {100 + (105 * i), 100},
                .size = {200, 200},
            });
            other_actor.set(components::Sprite{
                .spritesheet = gardner_sheet,
                .cutout = {
                    .position = {0, 64},
                    .size = {64, 64}
                }
            });
            other_actor.set(components::RenderData{
                .layer = RenderLayer::Objects
            });
            other_actor.add<components::SpriteRenderer>();
            other_actor.set(components::Collider{
                .is_trigger = true,
                .is_listening = false
            });
            other_actor.set(components::SquareCollider{
                .relative_size = {.5f, .9f}
            });
        }

        world.system<components::CollisionBegan, PlayerTest>("PlayerCollisionBegan")
            .term_at(0).second(flecs::Wildcard)
            .kind(flecs::OnUpdate)
            .each([&](flecs::iter& it, size_t, components::CollisionBegan, PlayerTest) {
                logger.info("Player collided with something!");
                auto other = it.pair(0).second();

                const auto* other_actor = other.get<components::Actor>();

                if (other_actor) {
                    logger.info("Player collided with actor {}", other_actor->id);
                }
            });

        world.system<components::CollisionEnded, PlayerTest>("PlayerCollisionEnded")
            .term_at(0).second(flecs::Wildcard)
            .kind(flecs::OnUpdate)
            .each([&](flecs::iter& it, size_t, components::CollisionEnded, PlayerTest) {
                logger.info("Player stopped colliding with something!");
                auto other = it.pair(0).second();

                const auto* other_actor = other.get<components::Actor>();

                if (other_actor) {
                    logger.info("Player stopped colliding with actor {}", other_actor->id);
                }
            });

        while (!main_window.ShouldClose()) {
            double current_time = Application::GetTime();
            double delta_time_d = current_time - last_time;
            
            if (delta_time_d < 1./60) {
                if (delta_time_d < 0.005) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                continue;
            }
            
            last_time = current_time;
            float delta_time = static_cast<float>(delta_time_d);

            main_screen_buffer.Use();
            Clear();
            glDisable(GL_DEPTH_TEST);
            draw_queues = {};

            auto input [[maybe_unused]] = ProcessInput(main_window.GetGlfwWindow());

            if (target_pos.has_value()) {
                const auto cam_view = camera.GetViewMatrix();
                const auto world_pos = glm::inverse(cam_view) * glm::vec4(*target_pos, 1, 1);

                logger.info("Moving to {}, {}", world_pos.x, world_pos.y);

                test_actor.set(components::DestinationIntent{.position = world_pos});
                test_actor.add<components::StaleDestination>();
                target_pos.reset();
            }

            camera.MoveBy(input.camera_movement * camera_speed * delta_time);

            world.progress();

            if (cycle_tiles) {
                SetTiles();
                cycle_tiles = false;
            }

            tile_grid_renderer.Render(draw_queues);
            room_renderer.Render(draw_queues);
            sprite_renderer.OnStore();

            draw_queues.RenderQueuedDraws();
            draw_queues.ClearQueuedDraws();

            main_screen_buffer.Render();

            dxgl::Screenbuffer::Unuse();

            if (toggle_debugger) {
                toggle_debugger = false;
                Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
            }

            rml_context->Update();
            rml_renderer.BeginFrame();
            rml_context->Render();
            rml_renderer.EndFrame();

            main_window.SwapBuffers();
            main_window.PollEvents();
        }

        Rml::Shutdown();
        Application::Terminate();
#if CATCH_EXCEPTIONS == 1
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
#endif    

}
