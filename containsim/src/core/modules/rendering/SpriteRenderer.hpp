#pragma once

#include <components/Transform.hpp>
#include <modules/rendering/RenderData.hpp>
#include <modules/rendering/Sprite.hpp>

#include <common/GlobalState.hpp>

namespace dxgl {
    struct Draw;
}

namespace rendering {
    class DrawQueues;
    class SpriteRendererSystem {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* p) const;
        };

        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

        DrawQueues* m_queues_out{};
    public:
        SpriteRendererSystem(dxgl::UboBindingManager& ubos, DrawQueues& queues);
        ~SpriteRendererSystem();

        void PreStore(
            const SpriteRenderer&,
            const components::Transform& transform,
            const RenderData& rdata, 
            const Sprite& sprite
        );

        void OnStore();

    private:
        dxgl::Draw MakeDrawTemplate() const;
    }; 
}