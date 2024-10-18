#pragma once

#include <variant>
#include <string>
#include <memory>
#include <unordered_map>

namespace services {
    struct UiNull { };
    struct UiUndefined { };
    struct UiObject;
    struct UiArray;

    // TODO: make objects and string views options here as well
    using UiValue = std::variant<UiUndefined, UiNull, bool, double, std::string, std::unique_ptr<UiObject>, std::unique_ptr<UiArray>>;

    struct UiObject {
        std::unordered_map<std::string, UiValue> fields{};
    };

    struct UiArray {
        std::vector<UiValue> elements{};
    };
}