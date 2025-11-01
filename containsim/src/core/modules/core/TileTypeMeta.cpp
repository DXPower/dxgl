#include <modules/core/TileTypeMeta.hpp>

#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

using namespace core;

TileTypeMetas::TileTypeMetas() {
    m_ids.AddId(0, "Nothing");
}

TileType TileTypeMetas::Add(TileTypeMeta meta) {
    int new_id = m_ids.AddId(meta.type);
    m_metas.try_emplace(new_id, std::move(meta));
    return new_id;
}

const TileTypeMeta* TileTypeMetas::Get(TileType id) const {
    auto it = m_metas.find(id);

    if (it == m_metas.end())
        return nullptr;

    return &it->second;
}

const TileTypeMeta* TileTypeMetas::Get(std::string_view type) const {
    auto id = GetIdOfType(type);

    if (id.has_value())
        return Get(*id);
    else
        return nullptr;
}

std::optional<TileType> TileTypeMetas::GetIdOfType(std::string_view type) const {
    const auto& id_map = m_ids.GetIds();

    auto it = id_map.right.find(std::string(type));

    if (it == id_map.right.end())
        return std::nullopt;

    return it->second;
}

std::generator<TileTypeMeta> core::LoadTileTypeMetasFromFile(flecs::world& world, std::string_view file) {
    std::ifstream json_file{std::string(file)};

    if (!json_file.is_open()) {
        throw std::runtime_error(std::format("Could not open file: {}", file));
    }

    nlohmann::json json_data = nlohmann::json::parse(json_file);

    auto scope_s = json_data["spritesheet_scope"].get<std::string>();
    auto name_s = json_data["spritesheet_entity"].get<std::string>();
    flecs::entity spritesheet_e = world.lookup(std::format("{}::{}", scope_s, name_s).c_str());
    auto spritesheet_file_s = json_data["spritesheet_path"].get<std::string>();

    if (!spritesheet_e) {
        auto texture = dxgl::LoadTextureFromFile(spritesheet_file_s);
        texture.SetFilterMode(dxgl::FilterMode::Nearest);

        spritesheet_e = world.entity(name_s.c_str())
            .set<dxgl::Texture>(std::move(texture));
    }
    
    dxgl::TextureView spritesheet_texture = spritesheet_e.get<dxgl::Texture>();

    for (const auto& meta_json : json_data["tiles"]) {
        TileTypeMeta meta{};
        meta.type = meta_json["type"].get<std::string>();
        meta.display_name = meta_json["display_name"].get<std::string>();
        
        auto layer_str = meta_json["layer"].get<std::string>();
        meta.layer = magic_enum::enum_cast<TileLayer>(layer_str).value();

        if (meta_json.contains("prefab")) {
            meta.prefab = meta_json["prefab"].get<std::string>();
        }

        meta.spritesheet = spritesheet_texture;
        meta.spritesheet_path = spritesheet_file_s;
        meta.sprite_origin.x = meta_json["x"].get<int>();
        meta.sprite_origin.y = meta_json["y"].get<int>();
        meta.sprite_size.x = meta_json["w"].get<int>();
        meta.sprite_size.y = meta_json["h"].get<int>();

        co_yield std::move(meta);
    }
}