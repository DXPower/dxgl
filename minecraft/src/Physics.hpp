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

    struct Aabb {
        glm::vec2 position{};
        glm::vec2 size{};

        Aabb Fattened(glm::vec2 pad) const;
    };

    struct AabbLineResult {
        glm::vec2 hit_position{};
        glm::vec2 penetration{};
        glm::vec2 surface_normal{};
        float time{};
    };

    struct AabbResult {
        glm::vec2 hit_position{};
        glm::vec2 penetration{};
        glm::vec2 surface_normal{};
    };

    struct SweptAabbResult {
        glm::vec2 hit_position{};
        glm::vec2 final_position{};
        glm::vec2 surface_normal{};
        float time{};
    };

    NearFar GetNearFarPoints(const Aabb& box, const Line& line);
    std::optional<AabbLineResult> TestAabbLineCollision(const Aabb& box, const Line& line);
    
    std::optional<std::array<AabbResult, 2>> TestAabbCollision(const Aabb& a, const Aabb& b);
    std::optional<SweptAabbResult> SweepAabbCollision(const Aabb& moving, const glm::vec2& vel, const Aabb& fixed);

    std::vector<SweptAabbResult> SweepChunk(const Aabb& moving, const glm::vec2& vel, const Chunk& chunk);

}