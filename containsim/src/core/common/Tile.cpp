#include <common/Tile.hpp>

#include <fstream>
#include <nlohmann/json.hpp>
#include <magic_enum/magic_enum.hpp>


std::map<TileType, TileMeta> LoadTileMetas() {
    std::map<TileType, TileMeta> metas{};

    std::ifstream json_file("res/spritesheets.json");
    nlohmann::json json_data = nlohmann::json::parse(json_file);

    for (const auto& sheet_json : json_data["sheets"]) {
        if (sheet_json["name"] == "tiles") {
            for (const auto& sprite_json : sheet_json["sprites"]) {
                auto name = sprite_json["name"].template get<std::string>();
                TileMeta meta{};
                
                if (auto casted = magic_enum::enum_cast<TileType>(name)) {
                    meta.type = casted.value();
                } else {
                    throw std::runtime_error("Invalid TileType in JSON file: " + name);
                }

                auto layer = sprite_json["layer"].template get<std::string>();
                if (auto casted = magic_enum::enum_cast<TileLayer>(layer)) {
                    meta.layer = casted.value();
                } else {
                    throw std::runtime_error("Invalid TileLayer in JSON file: " + layer);
                }

                meta.friendly_name = name;

                if (sprite_json.contains("prefab")) {
                    meta.prefab_name = sprite_json["prefab"].template get<std::string>();
                }

                meta.spritesheet_name = "tiles.tga";

                meta.sprite_origin = {
                    sprite_json["x"].template get<int>(),
                    sprite_json["y"].template get<int>()
                };

                meta.sprite_size = {
                    sprite_json["w"].template get<int>(),
                    sprite_json["h"].template get<int>()
                };

                metas.emplace(meta.type, meta);
            }
        }
    }

    return metas;
}

auto TileCoordNeighbors::ToList() const -> NeighborList {
    NeighborList list{};
    if (north) list.push_back(*north);
    if (northeast) list.push_back(*northeast);
    if (east) list.push_back(*east);
    if (southeast) list.push_back(*southeast);
    if (south) list.push_back(*south);
    if (southwest) list.push_back(*southwest);
    if (west) list.push_back(*west);
    if (northwest) list.push_back(*northwest);

    return list;
}