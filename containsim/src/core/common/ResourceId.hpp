#pragma once

#include <format>

enum class ResourceId : int { 
    Null = 0
};

template <>
struct std::formatter<ResourceId> : std::formatter<int> {
    auto format(ResourceId rid, format_context& ctx) const {
        return std::formatter<int>::format(static_cast<int>(rid), ctx);
    }
};
