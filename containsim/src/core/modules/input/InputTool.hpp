#pragma once

#include <modules/input/ToolEvent.hpp>
#include <modules/input/HotkeyMap.hpp>
#include <functional>

namespace input {

struct HandlerConfig {
    // When true, this handler will only be called when the tool is the leaf child
    bool only_when_active{};
};

class InputTool {
public:
    using Handler = std::function<void(InputTool&, const ToolEvent&, ToolEventControl&)>;

private:
    struct HandlerStorage {
        Handler handler_func{};
        HandlerConfig config{};
    };

    std::unordered_map<int, HandlerStorage> m_handlers{};
    HotkeyMap m_hotkeys{};

    InputTool* m_active_child{};
    InputTool* m_parent{};

public:
    InputTool() = default;

    virtual ~InputTool() = default;

    void On(int event_id, Handler handler, const HandlerConfig& config = {});
    // Returns true if the event was handled
    bool Dispatch(const ToolEvent& dispatch_event);
    
    void EnterChild(InputTool& child);
    void ExitToParent();
    void ReturnToSelf();

    bool HasChild() const;

    // Returns null if this tool has no parent
    InputTool* Parent() {
        return m_parent;
    }

    auto& Hotkeys(this auto&& self) {
        return self.m_hotkeys;
    }

    // If no child is active, returns self
    template<typename Self>
    std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const InputTool&, InputTool&>
    GetActiveChild(this Self&& self) {
        if (self.m_active_child) {
            return self.m_active_child->GetActiveChild();
        }

        return self;
    }

protected:
    virtual void OnEnter() { }
    virtual void OnExit() { }
    virtual void OnChildExit([[maybe_unused]] InputTool& child) { }
};

}