#include <common/Rect.hpp>

RectExtents GetRectExtents(const Rect& rect) {
    return RectExtents{
        .min = rect.position - (rect.size / 2.f),
        .max = rect.position + (rect.size / 2.f)
    };
}

Rect GetRectFromExtents(const RectExtents& extents) {
    return Rect{
        .position = (extents.min + extents.max) / 2.f,
        .size = extents.max - extents.min
    };
}

bool DoRectsIntersect(const RectExtents& a, const RectExtents& b) {
    return a.min.x < b.max.x && a.max.x > b.min.x &&
           a.min.y < b.max.y && a.max.y > b.min.y;
}