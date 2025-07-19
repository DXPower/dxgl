#include <common/Logging.hpp>
#include <modules/application/Application.hpp>
#include <modules/input/Input.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/rendering/Rendering.hpp>
#include <modules/core/Core.hpp>
#include <modules/ui/Ui.hpp>
#include <modules/experiment/Experiment.hpp>
#include <modules/misc_gameplay/MiscGameplay.hpp>

#include "DeltaTimer.hpp"

#include <dxgl/Application.hpp>
#include <flecs.h>

#include <exception>

#define CATCH_EXCEPTIONS

int main() {
    logging::SetDefaultLevel(logging::Level::debug);
    logging::SetCommonSink(logging::CreateConsoleSink());

    auto logger = logging::CreateLogger("main");

#ifdef CATCH_EXCEPTIONS
    try {
#endif        
        flecs::world world{};
        world.import<application::Application>();
        world.import<core::Core>();
        world.import<rendering::Rendering>();
        world.import<input::Input>();
        world.import<ui::Ui>();
        world.import<physics::Physics>();
        world.import<pathing::Pathing>();
        world.import<experiment::Experiment>();
        world.import<misc_gameplay::MiscGameplay>();

        DeltaTimer delta_timer{};
        auto& main_window = world.query<application::MainWindow>()
            .first().get_mut<dxgl::Window>();

        auto default_pipeline = world.get_pipeline();
        auto ticks_paused_pipeline = world.pipeline()
            .with(flecs::System)
            .without<core::DependsOnTicks>()
            .with(flecs::Phase).cascade(flecs::DependsOn)
            .without(flecs::Disabled).up(flecs::DependsOn)
            .without(flecs::Disabled).up(flecs::ChildOf)
            .build();

        auto tick_source = world.lookup("core::Core::TickSource");

        while (!main_window.ShouldClose()) {
            main_window.PollEvents();

            if (!tick_source.has<core::PauseTicks>()) {
                world.set_pipeline(default_pipeline);
            } else {
                world.set_pipeline(ticks_paused_pipeline);
            }

            auto delta_time = delta_timer.Tick();
            world.progress(static_cast<float>(delta_time));
        }

        Rml::Shutdown();
        dxgl::Application::Terminate();
#ifdef CATCH_EXCEPTIONS
    } catch (const std::exception& e) {
        logger.error("Unhandled exception: {}", e.what());
    }
#endif    

}
