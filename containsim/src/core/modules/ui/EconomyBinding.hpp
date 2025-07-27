#pragma once

#include <modules/core/Science.hpp>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Context.h>
#include <flecs.h>

namespace ui {

template<typename C, typename P>
class EcsVariableDefinition : public Rml::VariableDefinition {
public:
    flecs::query<> m_query{};
    P m_ptr{};

    EcsVariableDefinition() : EcsVariableDefinition({}, {}) { }
    EcsVariableDefinition(flecs::query<> q, P ptr) 
    : VariableDefinition(Rml::DataVariableType::Scalar)
    ,  m_query(std::move(q)), m_ptr(std::move(ptr)) { }

    bool Get(void*, Rml::Variant& variant) override {
        flecs::entity match = m_query.first();

        if (!match)
            return false;

        const auto& comp = match.get<C>();
        variant = std::invoke(m_ptr, comp);
        return true;
    }

    bool Set(void*, const Rml::Variant&) override {
        throw std::runtime_error("Setting UI->ECS variables is unsupported");
    }
};

class EconomyBinding {
    Rml::DataModelHandle m_model{};

    EcsVariableDefinition<core::TotalScience, decltype(&core::TotalScience::value)> m_science_var{};

public:
    EconomyBinding(Rml::Context& context, flecs::world& world) {
        Rml::DataModelConstructor constructor = context.CreateDataModel("economy");

        if (!constructor) {
            throw std::runtime_error("Failed to create data model economy");
        }

        flecs::query<> get_science = world.query_builder("UiGetScience")
            .with<core::TotalScience>()
            .build();

        m_science_var.m_query = std::move(get_science);
        m_science_var.m_ptr = &core::TotalScience::value;

        constructor.BindCustomDataVariable("total_science", Rml::DataVariable(&m_science_var, nullptr));

        m_model = constructor.GetModelHandle();

        world.observer()
            .with<core::TotalScience>().singleton()
            .event(flecs::OnSet)
            .each([this]() {
                m_model.DirtyVariable("total_science");
            });
    }
};

}
