#include <modules/input/ToolEvent.hpp>
#include <print>
using namespace core;
using namespace rendering;
using namespace input;

namespace {
glm::vec2 ScreenToWorldPos(glm::vec2 screen_pos, const Camera& camera) {
    const auto cam_view = camera.GetViewMatrix();
    const auto world_pos = glm::inverse(cam_view) * glm::vec4(screen_pos, 1, 1);
    return world_pos;
}
}

void ToolEvent::Builtin::InitIds(IdRegistry& registry) {
    Builtin::Cancel = registry.AddId("core.cancel");
    Builtin::Click = registry.AddId("core.click");
    Builtin::KeyPress = registry.AddId("core.key_press");
    Builtin::Drag = registry.AddId("core.drag");
    logging::CreateLogger("hi").info("{} {} {} {}", Cancel, KeyPress, Drag, Click);
}

ToolEventPayload input::MakePayloadFromAction(const Action& action, const Camera& camera, const TileGrid& tiles) {
    ToolEventPayload payload{};

    std::visit([&](const auto& data) {
        using T = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<T, KeyPress>) {
            payload = KeyPayload{data};
        } else if constexpr (std::is_same_v<T, MouseClick>) {
            ClickPayload click{};
            click.screen_pos = data.pos;
            click.world_pos = ScreenToWorldPos(data.pos, camera);
            click.tile_coord = tiles.WorldPosToTileCoord(click.world_pos);
            click.button = data.button;
            click.mods = data.mods;
            click.dir = data.dir;

            payload = std::move(click);
        }
    }, action.data);

    return payload;
}