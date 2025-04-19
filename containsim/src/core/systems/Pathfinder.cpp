#include <components/Pathing.hpp>
#include <components/Transform.hpp>
#include <systems/Pathfinder.hpp>
#include <glm/gtx/hash.hpp>
#include <queue>
#include <unordered_set>
#include <numbers>

using namespace systems;
using namespace components;

PathPoints Pathfinder::FindPath(const glm::vec2& start, const glm::vec2& end) const {
    auto start_tile = m_tile_grid->WorldPosToTileCoord(start);
    auto end_tile = m_tile_grid->WorldPosToTileCoord(end);    

    if (!start_tile || !end_tile) {
        return {}; // Invalid path
    }

    auto tile_path = AStar(start_tile.value(), end_tile.value());
    PathPoints path{};
    path.reserve(tile_path.size());
    
    for (const auto& tile : tile_path) {
        path.push_back(m_tile_grid->TileCoordToWorldPos(tile));
    }

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

namespace {
float Heuristic(const TileCoord& a, const TileCoord& b) {
    // Chebyshev distance
    return (float) std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}

// struct HeuristicComparator {
//     // In std::priority_map, using > here means the smallest heuristic score is popped first
//     bool operator()(const TileCoord& a, const TileCoord& b) const {
//         return Heuristic(a, b) > Heuristic(b, a);
//     }
// };

struct OpenSetTileCoord {
    TileCoord coord;
    float f_score;

    bool operator>(const OpenSetTileCoord& other) const {
        return f_score > other.f_score;
    }
};

std::vector<TileCoord> ReconstructPath(const std::unordered_map<TileCoord, TileCoord>& came_from, TileCoord current) {
    std::vector<TileCoord> total_path{current};
    
    while (came_from.contains(current)) {
        current = came_from.at(current);
        total_path.push_back(current);
    }

    std::ranges::reverse(total_path);
    return total_path;
}
}

std::vector<TileCoord> Pathfinder::AStar(const TileCoord& start, const TileCoord& end) const {
    std::priority_queue<OpenSetTileCoord, std::vector<OpenSetTileCoord>, std::greater<>> open_set{};
    std::unordered_set<TileCoord> open_set_unique{};
    open_set.push({start, Heuristic(start, end)});
    open_set_unique.insert(start);

    std::unordered_map<TileCoord, TileCoord> came_from{};
    
    std::unordered_map<TileCoord, float> g_score{};
    g_score[start] = 0;

    while (!open_set.empty()) {
        auto current = open_set.top();

        if (current.coord == end)
            return ReconstructPath(came_from, current.coord);

        open_set.pop();
        open_set_unique.erase(current.coord);

        for (auto neighbor : GetTileCoordNeighbors(*m_tile_grid, current.coord).ToList()) {
            if (m_tile_grid->GetTile(neighbor, TileLayer::Walls).type != TileType::Nothing) {
                continue; // Skip walls
            }

            // Diagonal movement cost is higher than straight movement
            const bool diagonal = glm::abs(neighbor - current.coord) == TileCoord{1, 1};
            constexpr float diagonal_cost = std::numbers::sqrt2; // sqrt(2)
            float neighbor_cost = diagonal ? diagonal_cost : 1.0f;
            auto tentative_g_score = g_score[current.coord] + neighbor_cost; // Assuming all edges have the same weight
            auto [it, inserted] = g_score.try_emplace(neighbor, 0); // Dummy 0 value

            if (inserted || tentative_g_score < it->second) {
                it->second = tentative_g_score;
                came_from[neighbor] = current.coord;

                if (!open_set_unique.contains(neighbor)) {
                    open_set.push({neighbor, tentative_g_score + Heuristic(neighbor, end)});
                    open_set_unique.insert(neighbor);
                }
            }
        }
    }

    return {}; // No path found
}