#pragma once

#include <components/Transform.hpp>
#include <components/RenderData.hpp>
#include <components/Sprite.hpp>

#include <common/GlobalState.hpp>

namespace dxgl {
    struct Draw;
}

class DrawQueues;

namespace systems {
    class SpriteRenderer {
        class Pimpl;
        std::unique_ptr<Pimpl> m_pimpl{};

        DrawQueues* m_queues_out{};
    public:
        SpriteRenderer(const GlobalState& global_state, DrawQueues& queues);
        ~SpriteRenderer();

        void PreStore(
            const components::SpriteRenderer&,
            const components::Transform& transform,
            const components::RenderData& rdata, 
            const components::Sprite& sprite
        );

        void OnStore();

    private:
        dxgl::Draw MakeDrawTemplate() const;
    }; 
}