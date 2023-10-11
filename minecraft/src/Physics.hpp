#pragma once

#include "Player.hpp"
#include "Block.hpp"

#include <span>
#include <glm/vec2.hpp>

namespace Physics {
    struct Collision {
        Block* block{};
        glm::vec2 penetration{};
        glm::vec2 surface_normal{};
        glm::vec2 hit_position{};
        float surface_area{};
    };

    struct Aabb {
        glm::vec2 position{};
        glm::vec2 size{};

        Aabb Fattened(glm::vec2 pad) const;
    };

    std::optional<std::array<Collision, 2>> TestAabbCollision(const Aabb& a, const Aabb& b);
    
    // void GetCollisions(
    //     Chunk& chunk,
    //     const Aabb& test,
    //     std::back_insert_iterator<std::vector<Block>> out);

    // // Returns the final position of a collision
    // glm::vec2 Resolve(const Aabb& test, glm::vec2 velocity, std::span<Chunk*> chunks);

}