#pragma once

#include <modules/input/InputTool.hpp>

namespace input {

class RootTool final : public InputTool {
    std::vector<std::unique_ptr<InputTool>> m_tools{};
    IdRegistry m_registry{};

    logging::Logger m_logger{logging::CreateLogger("RootTool")};
public:
    RootTool();
    RootTool(RootTool&&) noexcept= default;
    RootTool& operator=(RootTool&&) noexcept= default;

    void RegisterTool(std::unique_ptr<InputTool> tool, int event_to_activate);

    auto& Registry(this auto&& self) {
        return self.m_registry;
    }

protected:
    void OnChildExit(InputTool& child) override;
};

}