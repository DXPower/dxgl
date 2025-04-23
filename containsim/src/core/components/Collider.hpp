#pragma once

#include <components/ComponentProducer.hpp>
#include <glm/vec2.hpp>
#include <flecs.h>

#include <optional>

namespace components {
    struct Collider {
        // If true, this collider will not move nor move others in response to penetration
        bool is_trigger{};
        // If true, this collider will not move but may move others in response to penetration
        bool is_fixed{};
        // If true, CollisionResults will be added to this entity even if it is fixed or a trigger.
        // Additionally, CollisionBegan and CollisionEnded will only be added if this is true.
        bool is_listening{};
    };

    struct CollisionPenetration {
        glm::vec2 origin{};
        glm::vec2 penetration{};
    };

    // These relationship tags will all point to the entity it collided with
    struct CollisionBegan { };
    struct CollisionEnded { };
    struct CollisionWith {
        std::optional<CollisionPenetration> penetration{};
    };

    struct SquareCollider {
        glm::vec2 relative_size{1.f, 1.f};
        glm::vec2 offset{0.f, 0.f};
    };
}