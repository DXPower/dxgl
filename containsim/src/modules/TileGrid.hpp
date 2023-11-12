#pragma once

#include <common/Tile.hpp>

#include <memory>

#include <glm/vec2.hpp>
#include <flecs.h>

class DrawQueues;

namespace TileGrid {
    class Module {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* p) const;
        };

        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        Module(flecs::world& world);
        Module(Module&& move);
        ~Module();
        Module& operator=(Module&& move);

        void SetTile(const glm::ivec2& coord, const TileData& data);

        void Render(DrawQueues& draws) const;
    };
}