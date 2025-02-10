#pragma once

#include <services/InputStateEvents.hpp>
#include <services/EventManager.hpp>
#include <RmlUi/Core/DataModelHandle.h>

#include <magic_enum/magic_enum.hpp>

namespace services {
namespace ui {

class InputStateBinding {
    struct Data {
        InputStates cur_state{};
    } m_data{};
    Rml::DataModelHandle m_model{};

public:
    InputStateBinding(InputStates initial_state, Rml::Context& context, EventManager& em) {
        m_data.cur_state = initial_state;

        Rml::DataModelConstructor constructor = context.CreateDataModel("input_state");

        if (!constructor) {
            throw std::runtime_error("Failed to create data model input_state");
        }

        constructor.RegisterScalar<InputStates>(
            [](const InputStates& state, Rml::Variant& out) {
                out = std::string(magic_enum::enum_name(state));
            },
            [](InputStates& state, const Rml::Variant& in) {
                auto str = in.Get<std::string>();
                auto in_state = magic_enum::enum_cast<InputStates>(str);

                if (!in_state.has_value()) {
                    throw std::runtime_error("Failed to convert string to InputStates: " + str);
                }

                state = in_state.value();
            }
        );

        constructor.Bind("cur_state", &m_data.cur_state);

        m_model = constructor.GetModelHandle();

        em.GetOrRegisterSignal<InputStateChanged>()
            .signal.connect<&InputStateBinding::UpdateState>(this);
    }

private:
    void UpdateState(const InputStateChanged& event) {
        m_data.cur_state = event.to;
        m_model.DirtyVariable("cur_state");
    }
};

}
}