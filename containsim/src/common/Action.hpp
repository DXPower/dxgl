#pragma once

#include <cstdint>
#include <variant>

#include <glm/vec2.hpp>

enum class ButtonDir : uint8_t {
    Up, 
    Down,
    Repeat
};

struct KeyPress {
    ButtonDir dir{};
    int key{};
    int scancode{};
    int mods{};
};

struct TextInput {
    unsigned int codepoint{};
};

struct MouseMove {
    glm::dvec2 from{};
    glm::dvec2 to{};
};

struct MouseClick {
    ButtonDir dir{};
    glm::dvec2 pos{};
    int button{};
    int mods{};
};

struct ScrollInput {
    glm::dvec2 amount{};
};

struct Action {
    std::variant<KeyPress, TextInput, MouseMove, MouseClick, ScrollInput> data{};
};