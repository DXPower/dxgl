#pragma once

#include <string_view>
#include <RmlUi/Core/Event.h>

namespace ui_events {
    struct PanelEvent {
        std::string_view name{};
        enum class Type {
            Show,
            Hide
        } type{};
    };

    struct ElementEvent {
        const Rml::Event* event{};
        std::span<const std::string> args{};
    };
}