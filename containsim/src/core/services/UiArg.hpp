#pragma once

#include <variant>
#include <string>

namespace services {
    struct UiNull { };
    struct UiUndefined { };

    // TODO: make objects and string views options here as well
    using UiArg = std::variant<UiUndefined, UiNull, bool, double, std::string>;
}