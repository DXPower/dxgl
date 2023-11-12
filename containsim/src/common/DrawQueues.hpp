#pragma once

#include <common/Rendering.hpp>
#include <dxgl/Draw.hpp>

#include <queue>
#include <map>

#include <magic_enum_containers.hpp>

class DrawQueues {
    magic_enum::containers::array<RenderLayer, std::vector<dxgl::Draw>> m_owned_draws{};
    magic_enum::containers::array<RenderLayer, std::vector<const dxgl::Draw*>> m_viewed_draws{};

public:
    void QueueOwnedDraw(RenderLayer layer, dxgl::Draw&& draw);
    void QueueViewedDraw(RenderLayer layer, const dxgl::Draw& draw);

    void RenderQueuedDraws() const;
    void ClearQueuedDraws();
};