#pragma once

#include <modules/core/Tile.hpp>

#include <RmlUi/Core/DataModelHandle.h>
#include <magic_enum/magic_enum.hpp>

#include <ranges>

namespace ui {

class TilesBinding {
    struct Data {
        std::vector<core::TileMeta> metas{};
    } m_data{};

    Rml::DataModelHandle m_model{};

public:
    TilesBinding(Rml::Context& context) {
        using namespace core;

        m_data.metas = LoadTileMetas() | std::views::values | std::ranges::to<std::vector>();

        Rml::DataModelConstructor constructor = context.CreateDataModel("tiles");

        if (!constructor) {
            throw std::runtime_error("Failed to create data model tiles");
        }

        constructor.RegisterScalar<TileType>(
            detail::MakeEnumGetter<TileType>(),
            detail::MakeEnumSetter<TileType>()
        );

        auto ivec2_handle = constructor.RegisterStruct<glm::ivec2>();

        if (!ivec2_handle) {
            throw std::runtime_error("Failed to register struct glm::ivec2");
        }

        ivec2_handle.RegisterMember("x", &glm::ivec2::x);
        ivec2_handle.RegisterMember("y", &glm::ivec2::y);

        auto tile_meta_handle = constructor.RegisterStruct<TileMeta>();

        if (!tile_meta_handle) {
            throw std::runtime_error("Failed to register struct TileMeta");
        }

        tile_meta_handle.RegisterMember("type", &TileMeta::type);
        tile_meta_handle.RegisterMember("friendly_name", &TileMeta::friendly_name);
        tile_meta_handle.RegisterMember("spritesheet_name", &TileMeta::spritesheet_name);
        tile_meta_handle.RegisterMember("sprite_origin", &TileMeta::sprite_origin);
        tile_meta_handle.RegisterMember("sprite_size", &TileMeta::sprite_size);

        constructor.RegisterArray<decltype(m_data.metas)>();
        constructor.Bind("metas", &m_data.metas);

        m_model = constructor.GetModelHandle();
    }
};

}