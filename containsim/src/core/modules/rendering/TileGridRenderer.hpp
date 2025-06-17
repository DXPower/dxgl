#pragma once

#include <services/TileGrid.hpp>
#include <modules/rendering/DrawQueues.hpp>

#include <dxgl/Ubo.hpp>

#include <memory>
#include <glm/vec2.hpp>

namespace rendering {
    class TileGridRenderer {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* p) const;
        };

        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        TileGridRenderer(const services::TileGrid& tiles, dxgl::UboBindingManager& ubo_manager);
        ~TileGridRenderer();
        TileGridRenderer(TileGridRenderer&& move);
        TileGridRenderer& operator=(TileGridRenderer&& move);

        void Render(DrawQueues& draws) const;
    };
}