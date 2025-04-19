#pragma once

#include <common/Pathing.hpp>
#include <glm/vec2.hpp>

namespace components {
    struct DestinationIntent {
        glm::vec2 position{};
    };

    struct StaleDestination{};

    struct Path {
        PathPoints points{};
    };

    struct PathMover {
        std::size_t cur_node_idx{};
    };
}