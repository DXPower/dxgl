#include <modules/pathing/PathMover.hpp>
#include <modules/pathing/Pathing.hpp>
#include <modules/core/Transform.hpp>
#include <modules/core/Mobility.hpp>
#include <modules/core/Configuration.hpp>
#include <modules/core/Time.hpp>

#include <glm/gtx/norm.hpp>

using namespace pathing;

void pathing::PathMoverSystem(flecs::world& world) {
    auto s = world.system<Path, PathMover, const core::Mobility, core::Transform, const core::TickRate>("PathMover")
        .term_at<core::TickRate>().singleton()
        .kind(flecs::OnUpdate)
        .each([](flecs::iter& it, size_t row, Path& path, PathMover& mover, const core::Mobility& mobility, core::Transform& transform, const core::TickRate& tick_rate) {
            if (mover.cur_node_idx >= path.points.size()) {
                it.entity(row).remove<Path>();
                return;
            }

            const auto& target = path.points[mover.cur_node_idx];
            const auto delta = target - transform.position;
            const auto distance2 = glm::length2(delta);
            const auto tick_adjusted_speed = mobility.speed / tick_rate.period.count();

            if (distance2 <= (tick_adjusted_speed * tick_adjusted_speed * it.delta_time() * it.delta_time())) {
                transform.position = target;
                mover.cur_node_idx++;
            } else {
                // We don't need to calculate distance twice so just divide by the sqrt here
                transform.position += (delta / std::sqrt(distance2)) * tick_adjusted_speed * it.delta_time();
            }

            if (mover.cur_node_idx >= path.points.size()) {
                it.entity(row).remove<Path>();
            }
        });

    s.add<core::DependsOnTicks>();
}