#pragma once

#include <services/InputStateEvents.hpp>
#include <services/BuildInputEvents.hpp>
#include <services/EventManager.hpp>
#include <RmlUi/Core/DataModelHandle.h>

#include <magic_enum/magic_enum.hpp>

namespace services {
namespace ui {

namespace detail {
    template<typename T>
    static auto MakeEnumGetter() {
        return [](const T& e, Rml::Variant& out) {
            out = std::string(magic_enum::enum_name(e));
        };
    }

    template<typename T>
    static auto MakeEnumSetter() {
        return [](T& e, const Rml::Variant& in) {
            auto str = in.Get<std::string>();
            auto in_enum = magic_enum::enum_cast<T>(str);

            if (!in_enum.has_value()) {
                throw std::runtime_error("Failed to convert string to enum: " + str);
            }

            e = in_enum.value();
        };
    }
}

class InputStateBinding {
    struct Data {
        InputStates main_state{};
        BuildInputStates build_state{};
    } m_data{};
    Rml::DataModelHandle m_model{};

public:
    InputStateBinding(InputStates initial_state, Rml::Context& context, EventManager& em) {
        m_data.main_state = initial_state;

        Rml::DataModelConstructor constructor = context.CreateDataModel("input_state");

        if (!constructor) {
            throw std::runtime_error("Failed to create data model main_state");
        }

        constructor.RegisterScalar<InputStates>(
            detail::MakeEnumGetter<InputStates>(),
            detail::MakeEnumSetter<InputStates>()
        );

        constructor.RegisterScalar<BuildInputStates>(
            detail::MakeEnumGetter<BuildInputStates>(),
            detail::MakeEnumSetter<BuildInputStates>()
        );

        constructor.Bind("main_state", &m_data.main_state);
        constructor.Bind("build_state", &m_data.build_state);

        m_model = constructor.GetModelHandle();

        em.GetOrRegisterSignal<InputStateChanged>()
            .signal.connect<&InputStateBinding::UpdateState>(this);

        em.GetOrRegisterSignal<BuildInputStateChanged>()
            .signal.connect<&InputStateBinding::UpdateBuildState>(this);
    }

private:
    void UpdateState(const InputStateChanged& event) {
        m_data.main_state = event.to;
        m_model.DirtyVariable("main_state");
    }

    void UpdateBuildState(const BuildInputStateChanged& event) {
        m_data.build_state = event.to;
        m_model.DirtyVariable("build_state");
    }
};

}
}