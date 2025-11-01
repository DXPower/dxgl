#pragma once

#include <modules/core/Tile.hpp>
#include <modules/core/TileTypeMeta.hpp>

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Context.h>
#include <magic_enum/magic_enum.hpp>

#include <ranges>

namespace ui {

class TilesBinding {
    struct BoundMeta {
        core::TileType id{};
        core::TileTypeMeta meta{};
    };

    struct Data {
        std::vector<BoundMeta> metas{};
    } m_data{};

    Rml::DataModelHandle m_model{};

public:
    TilesBinding(Rml::Context& context, flecs::world& world) {
        using namespace core;

        const auto& metas_map = world.get<TileTypeMetas>().GetMetas();

        for (const auto& [k, v] : metas_map) {
            m_data.metas.emplace_back(k, v);
        }

        Rml::DataModelConstructor constructor = context.CreateDataModel("tile_types");

        if (!constructor) {
            throw std::runtime_error("Failed to create data model tiles");
        }

        auto ivec2_handle = constructor.RegisterStruct<glm::ivec2>();

        if (!ivec2_handle) {
            throw std::runtime_error("Failed to register struct glm::ivec2");
        }

        ivec2_handle.RegisterMember("x", &glm::ivec2::x);
        ivec2_handle.RegisterMember("y", &glm::ivec2::y);

        auto tile_type_meta_handle = constructor.RegisterStruct<TileTypeMeta>();

        if (!tile_type_meta_handle) {
            throw std::runtime_error("Failed to register struct TileTypeMeta");
        }

        tile_type_meta_handle.RegisterMember("type", &TileTypeMeta::type);
        tile_type_meta_handle.RegisterMember("friendly_name", &TileTypeMeta::display_name);
        tile_type_meta_handle.RegisterMember("spritesheet_path", &TileTypeMeta::spritesheet_path);
        tile_type_meta_handle.RegisterMember("sprite_origin", &TileTypeMeta::sprite_origin);
        tile_type_meta_handle.RegisterMember("sprite_size", &TileTypeMeta::sprite_size);

        auto bound_meta_handle = constructor.RegisterStruct<BoundMeta>();

        if (!bound_meta_handle) {
            throw std::runtime_error("Failed to register struct BoundMeta");
        }

        bound_meta_handle.RegisterMember("id", &BoundMeta::id);
        bound_meta_handle.RegisterMember("meta", &BoundMeta::meta);

        constructor.RegisterArray<decltype(m_data.metas)>();
        constructor.Bind("metas", &m_data.metas);

        m_model = constructor.GetModelHandle();
    }
};

}