#include <modules/input/HotkeyMap.hpp>
#include <common/Logging.hpp>

using namespace input;

void HotkeyMap::BindKey(int key, int action_id, ButtonDir dir) {
    auto [it, inserted] = m_key_bindings.insert_or_assign(Binding{key, dir}, action_id);

    if (!inserted) {
        logging::CreateLogger("HotkeyMap").warn("Overwriting existing binding for key {} dir {}", key, static_cast<int>(dir));
    }
}

void HotkeyMap::BindMouse(int button, int action_id, ButtonDir dir) {
    auto [it, inserted] = m_mouse_bindings.insert_or_assign(Binding{button, dir}, action_id);

    if (!inserted) {
        logging::CreateLogger("HotkeyMap").warn("Overwriting existing binding for button {} dir {}", button, static_cast<int>(dir));
    }
}

std::optional<int> HotkeyMap::TryTranslateKey(int key, ButtonDir dir) const {
    Binding binding{key, dir};
    
    if (auto it = m_key_bindings.find(binding); it != m_key_bindings.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

std::optional<int> HotkeyMap::TryTranslateMouse(int button, ButtonDir dir) const {
    Binding binding{button, dir};

    if (auto it = m_mouse_bindings.find(binding); it != m_mouse_bindings.end()) {
        return it->second;
    }

    return std::nullopt;
}