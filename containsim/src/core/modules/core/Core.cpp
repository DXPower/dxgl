#include <modules/core/Core.hpp>
#include <modules/application/Application.hpp>

#include <common/Ticks.hpp>

using namespace core;

namespace core {
    static void TickSystem(flecs::iter& it, size_t, EcsTickSource& flecs_timer, TimeSinceLastTick& time, const TickRate& rate); // NOLINT

    class TickController {
        static void SetNow(TickRep p) {
            TickClock::SetNow(TickClockKey{}, p);
        }

        static void Progress() {
            TickClock::Progress(TickClockKey{});
        }
    public:
        friend void core::TickSystem(flecs::iter& it, size_t, EcsTickSource& flecs_timer, TimeSinceLastTick& time, const TickRate& rate);
    };
}

void core::TickSystem(flecs::iter& it, size_t, EcsTickSource& flecs_timer, TimeSinceLastTick& time, const TickRate& rate) {
    time.value += ch::duration<float>(it.delta_time());
    
    flecs_timer.time_elapsed = time.value.count();

    // Tick whenever we reach the threshold
    if (time.value >= rate.period) {
        // Only subtract the period to account for overshoot
        time.value -= rate.period;
        flecs_timer.time_elapsed = time.value.count();
        flecs_timer.tick = true;

        TickController::Progress();
    }
}

Core::Core(flecs::world& world) {
    world.import<application::Application>();

    // Register core configurations
    core::RegisterCoreConfigurations(world);

    world.component<TimeSinceLastTick>();
    world.add<TimeSinceLastTick>();

    auto tick_source = world.timer();
    tick_source.stop();
    tick_source.set_name("TickSource");

    auto tick_system = world.system<EcsTickSource, TimeSinceLastTick, const TickRate>()
        .term_at(0).src(tick_source)
        .term_at(1).singleton()
        .term_at(2).singleton()
        .kind(flecs::PreUpdate)
        .each(&TickSystem);

    world.observer<const PauseTicks>()
        .term_at(0).src(tick_source)
        .event(flecs::OnAdd)
        .each([tick_system](const PauseTicks&) {
            tick_system.disable();
        });

    world.observer<const PauseTicks, TimeSinceLastTick>()
        .term_at(0).src(tick_source)
        .term_at(1).singleton().filter()
        .event(flecs::OnRemove)
        .each([tick_system](const PauseTicks&, TimeSinceLastTick& timer) {
            timer.value = {};
            tick_system.enable();
        });

    world.observer<const TickRate, TimeSinceLastTick>()
        .term_at(0).singleton()
        .term_at(1).singleton().filter()
        .event(flecs::OnSet)
        .each([](const TickRate&, TimeSinceLastTick& timer) {
            timer.value = {};
        });

    world.component<Actor>();
    world.component<Transform>();
    world.component<Mobility>();

    world.component<TileGrid>().add(flecs::Sparse);
    world.emplace<TileGrid>(world);

    auto& tile_grid = world.get_mut<TileGrid>();
    auto& event_manager = world.get_mut<application::EventManager>();

    world.component<BuildManager>().add(flecs::Sparse);
    world.emplace<BuildManager>(tile_grid, event_manager);

    world.component<RoomManager>().add(flecs::Sparse);
    world.emplace<RoomManager>(tile_grid, event_manager);
}