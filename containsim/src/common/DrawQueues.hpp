#pragma once

#include <common/Rendering.hpp>
#include <dxgl/Draw.hpp>

#include <vector>
#include <map>

struct DrawQueues {
    std::map<RenderLayer, std::vector<dxgl::Draw>> m_draw_queues{};

    void RenderQueues() const;
};