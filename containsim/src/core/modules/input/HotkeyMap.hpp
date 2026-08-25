#pragma once

#include <unordered_map>
#include <common/Action.hpp>
#include <boost/container_hash/hash.hpp>

namespace input {

class HotkeyMap {
    struct Binding {
        int which{};
        ButtonDir dir{};

        constexpr bool operator==(const Binding& rhs) const noexcept = default;
    };

    struct BindingHash {
        std::size_t operator()(const Binding& k) const noexcept {
            std::size_t seed = 0;
            boost::hash_combine(seed, k.which);
            boost::hash_combine(seed, static_cast<int>(k.dir));
            return seed;
        };
    };

    // Maps key+dir to action id
    std::unordered_map<Binding, int, BindingHash> m_key_bindings{};
    std::unordered_map<Binding, int, BindingHash> m_mouse_bindings{};

public:
    HotkeyMap() = default;

    void BindKey(int key, int action_id, ButtonDir = ButtonDir::Down);
    void BindMouse(int button, int action_id, ButtonDir = ButtonDir::Down);

    std::optional<int> TryTranslateKey(int key, ButtonDir dir) const;
    std::optional<int> TryTranslateMouse(int button, ButtonDir dir) const;
};

}