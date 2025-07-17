#pragma once

#include <RmlUi/Core/DataModelHandle.h>
#include <magic_enum/magic_enum.hpp>

namespace ui::detail {
    template<typename T>
    static auto MakeEnumGetter() {
        return [](const T& e, Rml::Variant& out) {
            out = std::string(magic_enum::enum_name(e));
        };
    }

    template<typename T>
    static auto MakeEnumSetter() {
        return [](T& e, const Rml::Variant& in) {
            auto str = in.Get<std::string>();
            auto in_enum = magic_enum::enum_cast<T>(str);

            if (!in_enum.has_value()) {
                throw std::runtime_error("Failed to convert string to enum: " + str);
            }

            e = in_enum.value();
        };
    }
}