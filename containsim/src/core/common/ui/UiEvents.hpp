#pragma once

#include <RmlUi/Core/Event.h>
#include <string_view>
#include <span>

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