#include "common/DrawQueues.hpp"
#include "common/Rendering.hpp"
#include <common/DrawQueues.hpp>

#include <magic_enum/magic_enum_utility.hpp>

void DrawQueues::QueueOwnedDraw(RenderLayer layer, dxgl::Draw&& draw) {
    m_owned_draws[layer].push_back(std::move(draw));
}

void DrawQueues::QueueViewedDraw(RenderLayer layer, const dxgl::Draw& draw) {
    m_viewed_draws[layer].push_back(&draw);
}


void DrawQueues::RenderQueuedDraws() const {
    magic_enum::enum_for_each<RenderLayer>([this](RenderLayer layer) {
        for (const auto& draw : m_owned_draws[layer]) {
            draw.Render();
        }

        for (const auto& draw : m_viewed_draws[layer]) {
            draw->Render();
        }
    });
}

void DrawQueues::ClearQueuedDraws() {
    magic_enum::enum_for_each<RenderLayer>([this](RenderLayer layer) {
        m_owned_draws[layer].clear();
        m_viewed_draws[layer].clear();
    });
}
