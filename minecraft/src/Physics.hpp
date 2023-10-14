#pragma once

#include "Player.hpp"
#include "Block.hpp"

#include <span>
#include <glm/vec2.hpp>

namespace Physics {
    struct NearFar {
        glm::vec2 near_x{}, near_y{};
        glm::vec2 near_times{};

        glm::vec2 far_x{}, far_y{};
        glm::vec2 far_times{};
    };

    struct Line {
        glm::vec2 from{};
        glm::vec2 to{};
    };

    struct Collision {
        Block* block{};
        glm::vec2 penetration{};
        glm::vec2 surface_normal{};
        glm::vec2 hit_position{};
    };

    struct Aabb {
        glm::vec2 position{};
        glm::vec2 size{};

        Aabb Fattened(glm::vec2 pad) const;
    };

    NearFar GetNearFarPoints(const Aabb& box, const Line& line);
    std::optional<Collision> TestAabbLineCollision(const Aabb& box, const Line& line);
    
    std::optional<std::array<Collision, 2>> TestAabbCollision(const Aabb& a, const Aabb& b);
    std::optional<Collision> SweepAabbCollision(const Aabb& moving, const glm::vec2& vel, const Aabb& fixed);

    // void GetCollisions(
    //     Chunk& chunk,
    //     const Aabb& test,
    //     std::back_insert_iterator<std::vector<Block>> out);

    // // Returns the final position of a collision
    // glm::vec2 Resolve(const Aabb& test, glm::vec2 velocity, std::span<Chunk*> chunks);

}