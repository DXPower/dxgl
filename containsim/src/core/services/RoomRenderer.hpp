#pragma once

#include <services/RoomManager.hpp>
#include <common/DrawQueues.hpp>

#include <dxgl/Ubo.hpp>

#include <memory>
#include <glm/vec2.hpp>

namespace services {
    class RoomRenderer {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* p) const;
        };

        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

    public:
        RoomRenderer(const RoomManager& room_manager, dxgl::UboBindingManager& ubo_manager);
        ~RoomRenderer();
        RoomRenderer(RoomRenderer&& move) noexcept;
        RoomRenderer& operator=(RoomRenderer&& move) noexcept;

        void Render(DrawQueues& draws) const;
    };
}