#pragma once

#include <common/Rendering.hpp>
#include <dxgl/Draw.hpp>

#include <magic_enum/magic_enum_containers.hpp>

namespace rendering {
    class DrawQueues {
        magic_enum::containers::array<RenderLayer, std::vector<dxgl::Draw>> m_owned_draws{};
        magic_enum::containers::array<RenderLayer, std::vector<const dxgl::Draw*>> m_viewed_draws{};

    public:
        DrawQueues() = default;
        DrawQueues(const DrawQueues&) = delete;
        DrawQueues(DrawQueues&&) = default;
        DrawQueues& operator=(const DrawQueues&) = delete;
        DrawQueues& operator=(DrawQueues&&) = default;
        
        void QueueOwnedDraw(RenderLayer layer, dxgl::Draw&& draw);
        void QueueViewedDraw(RenderLayer layer, const dxgl::Draw& draw);

        void RenderQueuedDraws() const;
        void ClearQueuedDraws();
    };
}