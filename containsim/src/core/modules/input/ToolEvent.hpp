#pragma once

#include <common/Action.hpp>
#include <common/IdRegistry.hpp>
#include <modules/core/TileCoord.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/rendering/Camera.hpp>
#include <glm/vec2.hpp>
#include <any>
#include <variant>

namespace input {

struct ClickPayload {
    glm::dvec2 screen_pos{};
    glm::vec2 world_pos{};
    std::optional<core::TileCoord> tile_coord{};
    int button{};
    int mods{};
    ButtonDir dir{};
};

struct KeyPayload : KeyPress { };

struct DragPayload {
    ClickPayload begin{};
    ClickPayload end{};
};

struct AnyPayload {
    std::any data{};
};

using ToolEventPayload = std::variant<
    std::monostate,
    ClickPayload,
    KeyPayload,
    DragPayload,
    AnyPayload
>;

namespace detail {
template<typename T, typename V>
struct IsVariantAlternative;

template<typename T, typename... Args>
struct IsVariantAlternative<T, std::variant<Args...>> : std::bool_constant<(std::is_same_v<T, Args> || ...)> { };

template<typename T, typename V>
concept IsVariantAlternative_v = IsVariantAlternative<T, V>::value;
}

template<typename T, typename P>
requires std::same_as<std::remove_cv_t<P>, ToolEventPayload>
auto* GetPayloadAs(P* payload) {
    if constexpr (detail::IsVariantAlternative_v<T, ToolEventPayload>) {
        return std::get_if<T>(payload);
    } else {
        auto* any_payload = std::get_if<AnyPayload>(payload);
        return any_payload ? std::any_cast<T>(&any_payload->data) : nullptr;
    }
}

template<typename T, typename P>
requires std::same_as<std::remove_cvref_t<P>, ToolEventPayload>
auto& GetPayloadAs(P&& payload) {
    if constexpr (detail::IsVariantAlternative_v<T, ToolEventPayload>) {
        return std::get<T>(payload);
    } else {
        auto& any_payload = std::get<AnyPayload>(payload);
        using C = std::conditional_t<std::is_const_v<std::remove_reference_t<P>>, const T&, T&>;
        return std::any_cast<C>(any_payload.data);
    }
}

struct ToolEvent {
    int id{};
    ToolEventPayload payload{};

    struct Builtin {
        inline static int Cancel{};
        inline static int Click{};
        inline static int KeyPress{};
        inline static int Drag{};

        static void InitIds(IdRegistry& registry);
    };
};

ToolEventPayload MakePayloadFromAction(const Action& action, const rendering::Camera& camera, const core::TileGrid& tiles);

class ToolEventControl {
    bool m_propagate{};

public:
    void PropagateUp() { m_propagate = true; };
    bool ShouldPropagate() const { return m_propagate; }
};

}