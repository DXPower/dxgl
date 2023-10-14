#include "Physics.hpp"

#include <cmath>
#include <iterator>
#include <optional>
#include <iostream>

using namespace Physics;

NearFar Physics::GetNearFarPoints(const Aabb& box, const Line& line) {
    const auto half_size = box.size / 2.f;
    const auto delta = line.to - line.from;
    const auto scale = glm::vec2(1.f, 1.f) / delta;
    const auto sign = glm::sign(scale);
    const auto near_times = (box.position - sign * half_size - line.from) * scale;
    const auto far_times = (box.position + sign * half_size - line.from) * scale;

    return NearFar{
        .near_x = line.from + delta * glm::vec2(near_times.x),
        .near_y = line.from + delta * glm::vec2(near_times.y),
        .near_times = near_times,
        .far_x = line.from + delta * glm::vec2(far_times.x),
        .far_y = line.from + delta * glm::vec2(far_times.y),
        .far_times = far_times
    };
}

std::optional<Collision> Physics::TestAabbLineCollision(const Aabb& box, const Line& line) {
    const auto near_far = GetNearFarPoints(box, line);

    if (near_far.near_times.x > near_far.far_times.y ||
        near_far.near_times.y > near_far.far_times.x) {
        return std::nullopt;
    }

    const auto max_near_time = std::max(near_far.near_times.x, near_far.near_times.y);
    const auto min_far_time = std::min(near_far.far_times.x, near_far.far_times.y);

    if (max_near_time >= 1 || min_far_time <= 0) {
        return std::nullopt;
    }


    const auto hit_time = std::clamp(max_near_time, 0.f, 1.f);
    const auto line_delta = line.to - line.from;
    const auto sign = glm::sign(line_delta);

    Collision collision{};
    collision.hit_position = line.from + line_delta * hit_time;
    collision.penetration = -line_delta * (1.f - hit_time);
    collision.surface_normal = near_far.near_times.x > near_far.near_times.y ?
        glm::vec2{-sign.x, 0} :
        glm::vec2{0, -sign.y};

    return collision;
}

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

// std::optional<Collision> TestSweptAabbCollision(const Aabb& moving, const glm::vec2& vel, const Aabb& fixed) {
//     if (vel == glm::vec2{0, 0}) {
//         auto res = TestAabbCollision(moving, fixed);
//         if (res.has_value()) {
//             return res->at(0);
//         } else {
//             return std::nullopt;
//         }
//     }


// }

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