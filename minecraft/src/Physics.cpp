#include "Physics.hpp"

#include <cmath>
#include <iterator>
#include <optional>

using namespace Physics;

// Returns two possible collision resolutions
std::optional<std::array<Collision, 2>> Physics::TestAabbCollision(const Aabb& a, const Aabb& b) {
    const auto delta = b.position - a.position;
    const auto penetration = (a.size + b.size) / 2.f - glm::abs(delta);   

    if (penetration.x <= 0 || penetration.y <= 0) {
        return std::nullopt;
    }

    std::array<Collision, 2> resolutions{};
    const auto sign = glm::sign(delta);

    resolutions[0].penetration.x = penetration.x * sign.x;
    resolutions[0].surface_normal.x = sign.x;
    resolutions[0].hit_position = { a.position.x + ( a.position.x / 2 * sign.x), b.position.y };

    resolutions[1].penetration.y = penetration.y * sign.y;
    resolutions[1].surface_normal.y = sign.y;
    resolutions[1].hit_position = { b.position.x, a.position.y + ( a.position.y / 2 * sign.y) };

    return resolutions;
}
    // void ResolveCollision(Player& player, const Collision& collision);

Aabb Physics::Aabb::Fattened(glm::vec2 pad) const {
    Aabb copy = *this;
    copy.size += pad;
    return copy;
}

// void Physics::GetCollisions(
//     Chunk& chunk,
//     const Aabb& test,
//     std::back_insert_iterator<std::vector<Block>> out) {
    
//     for (Block& block : chunk.blocks) {

//     }
// }

// glm::vec2 Physics::Resolve(const Aabb& test, std::span<Chunk*> chunks) {

// }