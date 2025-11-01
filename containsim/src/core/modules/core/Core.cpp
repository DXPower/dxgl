#include <modules/core/Core.hpp>
#include <modules/application/Application.hpp>

#include <common/Ticks.hpp>
#include <dxgl/Texture.hpp>

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
        // If we've overshot by multiple ticks, just reset back to 0
        // This is particularly useful when debugging so if we pause for a while,
        // we don't throw out thousands of ticks at once
        if (time.value > rate.period) {
            time.value = {};
        }

        flecs_timer.time_elapsed = time.value.count();
        flecs_timer.tick = true;

        TickController::Progress();
    }
}

Core::Core(flecs::world& world) {
    auto logger = logging::CreateSharedLogger("Core");

    world.import<application::Application>();

    // Register core configurations
    core::RegisterCoreConfigurations(world);
    world.component<DependsOnTicks>();
    world.component<PauseTicks>();
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
    world.component<dxgl::Texture>().add(flecs::Sparse);

    world.component<TileTypeMetas>().add(flecs::Sparse);
    world.add<TileTypeMetas>();
    
    world.component<TileGrid>().add(flecs::Sparse);
    world.emplace<TileGrid>(world);

    auto& tile_grid = world.get_mut<TileGrid>();
    auto& event_manager = world.get_mut<application::EventManager>();

    world.component<RoomManager>().add(flecs::Sparse);
    world.emplace<RoomManager>(tile_grid, event_manager);

    world.component<RoomTypeMetas>().add(flecs::Sparse);
    world.add<RoomTypeMetas>();

    auto& room_type_metas = world.get_mut<RoomTypeMetas>();

    for (auto&& meta : LoadRoomTypeMetasFromFile("res/rooms.json")) {
        room_type_metas.Add(std::move(meta));
    }

    auto& tile_type_metas = world.get_mut<TileTypeMetas>();

    for (auto&& meta : LoadTileTypeMetasFromFile(world, "res/tiles.json")) {
        tile_type_metas.Add(std::move(meta));
    }

    world.component<BuildManager>().add(flecs::Sparse);
    world.emplace<BuildManager>(tile_grid, tile_type_metas, event_manager);

    world.component<Cooldown>();
    world.system<Cooldown>()
        .tick_source(tick_source)
        .each([](flecs::entity e, Cooldown& c) {
            c.time_remaining--;

            if (c.time_remaining.count() == 0) {
                e.remove<Cooldown>();
            }
        })
        .add<core::DependsOnTicks>();


    SetupInteractionSystems(world);

    world.component<TotalScience>();
    world.add<TotalScience>();
    world.component<ScienceGain>();

    world.system<const ScienceGain, TotalScience>()
        .term_at<TotalScience>().singleton()
        .write<TotalScience>()
        .kind(flecs::PostUpdate)
        .each([logger](flecs::entity e, const ScienceGain& g, TotalScience& t) {
            t.value += g.value;
            e.remove<ScienceGain>();

            logger->debug("Gained {} science, now have {} total", g.value, t.value);

            e.world().modified<TotalScience>();
        });
}