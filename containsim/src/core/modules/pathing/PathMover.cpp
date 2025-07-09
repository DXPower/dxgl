#include <modules/pathing/PathMover.hpp>
#include <modules/pathing/Pathing.hpp>
#include <components/Transform.hpp>
#include <components/Mobility.hpp>

#include <glm/gtx/norm.hpp>

using namespace pathing;

void pathing::PathMoverSystem(flecs::world& world) {
    world.system<Path, PathMover, const components::Mobility, components::Transform>("PathMover")
        .kind(flecs::OnUpdate)
        .each([](flecs::iter& it, size_t row, Path& path, PathMover& mover, const components::Mobility& mobility, components::Transform& transform) {
            if (mover.cur_node_idx >= path.points.size()) {
                it.entity(row).remove<Path>();
                return;
            }

            const auto& target = path.points[mover.cur_node_idx];
            const auto delta = target - transform.position;
            const auto distance2 = glm::length2(delta);

            if (distance2 <= (mobility.speed * mobility.speed * it.delta_time() * it.delta_time())) {
                transform.position = target;
                mover.cur_node_idx++;
            } else {
                // We don't need to calculate distance twice so just divide by the sqrt here
                transform.position += (delta / std::sqrt(distance2)) * mobility.speed * it.delta_time();
            }

            if (mover.cur_node_idx >= path.points.size()) {
                it.entity(row).remove<Path>();
            }
        });
}