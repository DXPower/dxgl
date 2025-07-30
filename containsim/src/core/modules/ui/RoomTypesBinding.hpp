#pragma once

#include <modules/core/RoomTypeMeta.hpp>

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Context.h>
#include <magic_enum/magic_enum.hpp>
#include <flecs.h>

#include <ranges>

namespace ui {

class RoomTypesBinding {
    struct BoundMeta {
        core::RoomType id{};
        core::RoomTypeMeta meta{};
    };

    struct Data {
        std::vector<BoundMeta> metas{};
    } m_data{};

    Rml::DataModelHandle m_model{};

public:
    RoomTypesBinding(Rml::Context& context, flecs::world& world) {
        using namespace core;

        const auto& metas_map = world.get<RoomTypeMetas>().GetMetas();

        m_data.metas.reserve(metas_map.size());

        for (const auto& [k, v] : metas_map) {
            m_data.metas.emplace_back(k, v);
        }

        Rml::DataModelConstructor constructor = context.CreateDataModel("room_types");
        
        if (!constructor) {
            throw std::runtime_error("Failed to create data model room_types");
        }

        auto room_type_meta_handle = constructor.RegisterStruct<RoomTypeMeta>();

        if (!room_type_meta_handle) {
            throw std::runtime_error("Failed to register struct RoomTypeMeta");
        }

        room_type_meta_handle.RegisterMember("type", &RoomTypeMeta::type);
        room_type_meta_handle.RegisterMember("display_name", &RoomTypeMeta::display_name);

        auto bound_meta_handle = constructor.RegisterStruct<BoundMeta>();

        if (!bound_meta_handle) {
            throw std::runtime_error("Failed to register struct BoundMeta");
        }

        bound_meta_handle.RegisterMember("id", &BoundMeta::id);
        bound_meta_handle.RegisterMember("meta", &BoundMeta::meta);

        constructor.RegisterArray<decltype(m_data.metas)>();
        constructor.Bind("bound_metas", &m_data.metas);

        m_model = constructor.GetModelHandle();
    }
};

}