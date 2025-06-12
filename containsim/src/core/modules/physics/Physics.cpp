#include <modules/physics/Physics.hpp>

#include <modules/physics/Collider.hpp>
#include <components/Transform.hpp>

#include <common/DebugDraws.hpp>

using namespace physics;
using namespace components;

namespace {
auto SquareColliderSystem(flecs::world& world) {
    auto square_colliders = world.query<const Collider, const SquareCollider, const Transform>("SquareColliderQuery");

    return [square_colliders = std::move(square_colliders)](flecs::iter&) {
        square_colliders.each([&](flecs::entity ae, const Collider& ac, const SquareCollider& asq, const Transform& at) {
            auto a_col_rect = at.ToRect();
            a_col_rect.position += asq.offset;
            a_col_rect.size *= asq.relative_size;

            const auto a_col_extents = GetRectExtents(a_col_rect);

            std::array<glm::vec2, 5> points = {
                a_col_extents.min,
                {a_col_extents.max.x, a_col_extents.min.y},
                a_col_extents.max,
                {a_col_extents.min.x, a_col_extents.max.y},
                a_col_extents.min
            };

            DebugDraws::MakeWorldDraw(points, glm::vec4(0, 1, 0, 1), dxgl::PrimType::LineStrip);

            square_colliders.each([&](flecs::entity be, const Collider& bc, const SquareCollider& bsq, const Transform& bt) {
                // Skip collisions that have been checked already,
                if (ae >= be) {
                    return;
                }

                auto b_col_rect = bt.ToRect();
                b_col_rect.position += bsq.offset;
                b_col_rect.size *= bsq.relative_size;
                const auto b_col_extents = GetRectExtents(b_col_rect);

                if (!DoRectsIntersect(a_col_extents, b_col_extents)) {
                    // Collision ended events
                    if (ae.has<CollisionWith>(be)) {
                        if (ac.is_listening) {
                            ae.add<CollisionEnded>(be);
                        }

                        ae.remove<CollisionWith>(be);
                    }

                    if (bc.is_listening && be.has<CollisionWith>(ae)) {
                        if (bc.is_listening) {
                            be.add<CollisionEnded>(ae);
                        }
                        
                        be.remove<CollisionWith>(ae);
                    }

                    return; // No collision
                }

                // Collision began events
                if (ac.is_listening && !ae.has<CollisionWith>(be)) {
                    ae.add<CollisionBegan>(be);
                }

                if (bc.is_listening && !be.has<CollisionWith>(ae)) {
                    be.add<CollisionBegan>(ae);
                }

                // Collision detected, create event
                // Ignore penetration for now
                CollisionWith result_for_a{};
                CollisionWith result_for_b{};

                // Create the relationship (CollisionWith, Entity) pointing to each other
                ae.set(be, result_for_a);
                be.set(ae, result_for_b);
            });
        });
    };
}
}

Physics::Physics(flecs::world& world) {
    world.component<Collider>();
    world.component<CollisionBegan>();
    world.component<CollisionEnded>();
    world.component<CollisionWith>();
    world.component<SquareCollider>();

    // PreUpdate systems: Detect collisions and dispatch events
    // PostUpdate systems: Cleanup events
   
    world.system("SquareColliderCollision")
        .kind(flecs::PreUpdate)
        .run(SquareColliderSystem(world));
    
    world.system<CollisionBegan>("CollisionBeganCleanup")
        .kind(flecs::PostUpdate)
        .term_at(0).second(flecs::Wildcard)
        .each([](flecs::iter& it, size_t row, CollisionBegan) {
            auto e = it.entity(row);
            auto p = it.pair(0);
            e.remove<CollisionBegan>(p.second());
        });

    world.system<CollisionEnded>("CollisionEndedCleanup")
        .kind(flecs::PostUpdate)
        .term_at(0).second(flecs::Wildcard)
        .each([](flecs::iter& it, size_t row, CollisionEnded) {
            auto e = it.entity(row);
            auto p = it.pair(0);
            e.remove<CollisionEnded>(p.second());
        });
}