#pragma once

#include <common/Tile.hpp>

#include <memory>

#include <glm/vec2.hpp>
#include <flecs.h>

class DrawQueues;

namespace TileGrid {
    class Module {
        class Pimpl;
        std::unique_ptr<Pimpl> m_pimpl{};

    public:
        Module(flecs::world& world);
        Module(Module&& move);
        ~Module();
        Module& operator=(Module&& move);

        void SetTile(const glm::ivec2& coord, const TileData& data);

        void Render(DrawQueues& draws) const;
    };
}