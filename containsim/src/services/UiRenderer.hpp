#pragma once

#include <dxtl/cstring_view.hpp>

#include <memory>
#include <string_view>

class DrawQueues;

namespace services {
    class UiRenderer {
        class Pimpl;
        std::unique_ptr<Pimpl> m_pimpl{};

    public:
        UiRenderer();
        ~UiRenderer();

        void Update();
        void Render(DrawQueues& draw_queues) const;

        void LoadHtml(std::string_view path);
        void LoadCss(std::string_view path); 

        void LoadUrl(dxtl::cstring_view path);
    };
}