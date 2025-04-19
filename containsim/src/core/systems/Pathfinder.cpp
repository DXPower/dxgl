#include <components/Pathing.hpp>
#include <components/Transform.hpp>
#include <systems/Pathfinder.hpp>

using namespace systems;
using namespace components;

PathPoints Pathfinder::FindPath(const glm::vec2& start, const glm::vec2& end) const {
    // Basic algorithm for now - straight line
    PathPoints path{};
    path.push_back(start);
    path.push_back(end);
    return path;
}

void Pathfinder::PreUpdate(flecs::world& world) const {
    world.system<StaleDestination, const DestinationIntent, const Transform>("Pathfinder")
        .kind(flecs::PreUpdate)
        .each([this](flecs::entity e, StaleDestination, const DestinationIntent& intent, const Transform& transform) {
            auto path = FindPath(transform.position, intent.position);
            e.set<Path>(Path{.points = std::move(path)});
            e.remove<StaleDestination>();

            // Reset the path mover to look at the start of the new path if we update it
            if (auto* path_mover = e.get_mut<PathMover>()) {
                path_mover->cur_node_idx = 1;
            }
        });
}