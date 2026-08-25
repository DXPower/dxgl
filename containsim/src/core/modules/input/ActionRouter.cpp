#include <modules/input/ActionRouter.hpp>

#include <dxtl/overloaded.hpp>
#include <magic_enum/magic_enum_containers.hpp>

using namespace input;

ActionRouter::ActionRouter(const dxgl::Window& window, RootTool& root_tool, const rendering::Camera& camera, const core::TileGrid& tiles)
    : m_mouse_tester(window)
    , m_root_tool(&root_tool)
    , m_camera(&camera)
    , m_tiles(&tiles)
{ }

void ActionRouter::Consume(Action&& action) {
    magic_enum::containers::array<InputLayer, bool> routes{};
    std::optional<glm::dvec2> mouse_pos{};

    bool update_click_start = false;

    std::visit([&]<typename T>(const T& a) {
        if constexpr (std::is_same_v<T, MouseClick>) {
            const MouseClick& click = a;

            mouse_pos = click.pos;

            if (click.dir == ButtonDir::Down) {
                update_click_start = true;
            } else if (click.dir == ButtonDir::Up) {
                // Notify where the click started that the click has stopped
                routes[m_click_start] = true;
            }
        } else if constexpr (std::is_same_v<T, MouseMove>) {
            mouse_pos = a.to;

            // Mouse movements should broadcast to everyone
            for (bool& route : routes) {
                route = true;
            }
        } else if constexpr (std::is_same_v<T, ScrollInput>) {
            mouse_pos = a.pos;
        } else if constexpr (std::is_same_v<T, KeyPress>) {
            // TODO: Check for keyboard focus in UI
            routes[InputLayer::Game] = true;
        }
    }, action.data);

    if (mouse_pos.has_value()) {
        const InputLayer route = m_mouse_tester.TestMouse(*mouse_pos);
        routes[route] = true;

        if (update_click_start)
            m_click_start = route;
    }

    if (routes[InputLayer::Game] || routes[InputLayer::Offscreen]) {

        auto id = std::visit([]<typename T>(const T&) -> std::optional<int> {
            if constexpr (std::same_as<T, KeyPress>) {
                return ToolEvent::Builtin::KeyPress;
            } else if constexpr (std::same_as<T, MouseClick>) {
                return ToolEvent::Builtin::Click;
            }

            return std::nullopt;
        }, action.data);

        bool handled_by_tool = false;

        if (id.has_value()) {
            ToolEvent event{};
            event.id = *id;
            event.payload = MakePayloadFromAction(action, *m_camera, *m_tiles);
            handled_by_tool = m_root_tool->Dispatch(event);
        }

        if (!handled_by_tool) {
            global_action_producer.Send(std::move(action));
        }
        // auto* sub_fsm = m_actions_out->GetActiveSubFsm();
        // global_action_producer.Send(std::move(action));

        // if (sub_fsm == nullptr) {
        //     return;
        // }

        // const auto& events = sub_fsm->GetEventInfo().GetIds();
        // auto action_id_it = events.right.find("Action");

        // if (action_id_it == events.right.end()) {
        //     m_logger.warn("Expected to send this Action somewhere, but {} can't accept it", sub_fsm->GetName());
        //     return;
        // }

        // sub_fsm->GetFsm().InsertEvent(action_id_it->second, std::move(action));
    }
}