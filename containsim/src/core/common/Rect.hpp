#pragma once

#include <glm/vec2.hpp>

struct Rect {
    glm::vec2 position{};
    glm::vec2 size{};
};

struct RectExtents {
    glm::vec2 min{};
    glm::vec2 max{};
};

RectExtents GetRectExtents(const Rect& rect);
Rect GetRectFromExtents(const RectExtents& extents);

bool DoRectsIntersect(const RectExtents& a, const RectExtents& b);