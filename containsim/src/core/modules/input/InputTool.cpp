#include <modules/input/InputTool.hpp>
#include <common/Logging.hpp>

using namespace input;

namespace {
    thread_local std::optional<logging::Logger> logger{};

    logging::Logger& GetLogger() {
        if (!logger.has_value()) {
            logger = logging::CreateLogger("InputTool");
        }
        return *logger;
    }
}

void InputTool::On(int event_id, Handler handler, const HandlerConfig& config) {
    auto [it, inserted] = m_handlers.emplace(event_id, HandlerStorage{std::move(handler), config});

    if (!inserted) {
        GetLogger().warn("Handler for event {} already exists, overwriting", event_id);
    }
}

bool InputTool::Dispatch(const ToolEvent& dispatch_event) {
    bool handled = false;

    // Start with the deepest active child and climb up
    for (InputTool* probe = &GetActiveChild(); probe != nullptr; probe = probe->Parent()) {
        // Try to convert this as a hotkey 
        std::optional<int> hotkey_translation{};

        if (dispatch_event.id == ToolEvent::Builtin::KeyPress) {
            const auto& kp = GetPayloadAs<KeyPayload>(dispatch_event.payload);
            hotkey_translation = probe->Hotkeys().TryTranslateKey(kp.key, kp.dir);
        } else if (dispatch_event.id == ToolEvent::Builtin::Click) {
            const auto& c = GetPayloadAs<MousePayload>(dispatch_event.payload);
            hotkey_translation = probe->Hotkeys().TryTranslateMouse(c.button, c.dir);
        }
        
        if (hotkey_translation.has_value()) {
            // Restart the Dispatch using the new event.
            // Don't attempt to continue bubbling after translation.
            // Notably - return true only if the translated event is handled.
            // (A hotkey that maps to an unhandled event is considered unhandled itself)
            return Dispatch(ToolEvent{
                .id = *hotkey_translation
            }) || handled;
        }

        // If it cannot be translated as a hotkey, check if the child has a handler for the event.
        auto handler_it = probe->m_handlers.find(dispatch_event.id);

        if (handler_it == probe->m_handlers.end()) {
            // This child cannot handle this event
            continue;
        }

        // The child can attempt to handle it
        ToolEventControl control{};
        auto& storage = handler_it->second;

        if (storage.config.only_when_active && probe->HasChild()) {
            continue;
        }

        storage.handler_func(*probe, dispatch_event, control);
        handled = true;

        if (!control.ShouldPropagate()) {
            // The event is fully handled, end bubbling.
            break;
        }

        // Stop bubbling once we reach the original node
        if (probe == this) {
            break;
        }
    }

    return handled;
}

void InputTool::EnterChild(InputTool& child) {
    ReturnToSelf();
    m_active_child = &child;
    m_active_child->m_parent = this;
    m_active_child->OnEnter();
}

void InputTool::ExitToParent() {
    if (m_parent == nullptr) {
        GetLogger().error("Attempted to exit to parent, but no parent exists");
        return;
    }

    m_parent->ReturnToSelf();
}

void InputTool::ReturnToSelf() {
    InputTool* active_child = &GetActiveChild();

    while (active_child != this) {
        active_child->OnExit();
        // Clear the parent's active child
        active_child->m_parent->m_active_child = nullptr;
        // Grab the parent as the next child, and clear the previous child's parent.
        InputTool& previous_child = *active_child;
        active_child = std::exchange(active_child->m_parent, nullptr);
        active_child->OnChildExit(previous_child);
    }
}

bool InputTool::HasChild() const {
    return m_active_child != nullptr;
}