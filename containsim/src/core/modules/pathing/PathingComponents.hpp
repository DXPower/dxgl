#pragma once

#include <modules/pathing/Paths.hpp>

namespace pathing {
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