#include <modules/core/Tile.hpp>

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <magic_enum/magic_enum.hpp>

using namespace core;

std::map<TileType, TileMeta> core::LoadTileMetas() {
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

bool TileSelection::Contains(const TileCoord& coord) const {
    auto [x_min, x_max] = std::minmax(start.x, end.x);
    auto [y_min, y_max] = std::minmax(start.y, end.y);

    return coord.x >= x_min && coord.x <= x_max &&
           coord.y >= y_min && coord.y <= y_max;
}

std::generator<TileCoord> TileSelection::Iterate() const {
    int x_step = start.x < end.x ? 1 : -1;
    int y_step = start.y < end.y ? 1 : -1;

    for (auto x = start.x; x != end.x + x_step; x += x_step) {
        for (auto y = start.y; y != end.y + y_step; y += y_step) {
            co_yield TileCoord{x, y};
        }
    }
}

int TileSelection::Area() const {
    // Add 1 because end is inclusive
    int w = std::abs(start.x - end.x) + 1;
    int h = std::abs(start.y - end.y) + 1;
    return w * h;
}