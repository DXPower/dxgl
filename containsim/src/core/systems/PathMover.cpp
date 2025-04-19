#include <systems/PathMover.hpp>
#include <components/Pathing.hpp>
#include <components/Transform.hpp>
#include <components/Mobility.hpp>
#include <common/DebugDraws.hpp>

#include <glm/gtx/norm.hpp>
#include <cmath>

void systems::PathMover(flecs::world& world) {
    world.system<components::Path, components::PathMover, const components::Mobility, components::Transform>("PathMover")
        .kind(flecs::OnUpdate)
        .each([](flecs::iter& it, size_t row, components::Path& path, components::PathMover& mover, const components::Mobility& mobility, components::Transform& transform) {
            if (mover.cur_node_idx >= path.points.size()) {
                it.entity(row).remove<components::Path>();
                return;
            }

            const auto& target = path.points[mover.cur_node_idx];
            const auto delta = target - transform.position;
            const auto distance2 = glm::length2(delta);

            DebugDraws::MakeWorldDraw(path.points, {1.f, 0.f, 0.f, 1.f}, dxgl::PrimType::Line);

            if (distance2 <= (mobility.speed * mobility.speed * it.delta_time() * it.delta_time())) {
                transform.position = target;
                mover.cur_node_idx++;
            } else {
                // We don't need to calculate distance twice so just divide by the sqrt here
                transform.position += (delta / std::sqrt(distance2)) * mobility.speed * it.delta_time();
            }

            if (mover.cur_node_idx >= path.points.size()) {
                it.entity(row).remove<components::Path>();
            }
        });
}