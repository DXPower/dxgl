#pragma once

#include <modules/input/InputTool.hpp>
#include <modules/input/DragTracker.hpp>

namespace input {

class RootTool final : public InputTool {
    std::vector<std::unique_ptr<InputTool>> m_tools{};
    IdRegistry m_registry{};

    DragTracker m_drag_tracker{};

    logging::Logger m_logger{logging::CreateLogger("RootTool")};
public:
    RootTool();
    RootTool(RootTool&&) noexcept= default;
    RootTool& operator=(RootTool&&) noexcept= default;

    void RegisterTool(std::unique_ptr<InputTool> tool, int event_to_activate);
    void UpdateMousePos(const MousePayload& payload);

    auto& Registry(this auto&& self) {
        return self.m_registry;
    }

protected:
    void OnChildExit(InputTool& child) override;

private:
    void TryControlDrag(InputTool&, const ToolEvent& ev, ToolEventControl&);
};

}