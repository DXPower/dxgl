#include <modules/core/StaffTypeMeta.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

using namespace core;

std::generator<StaffTypeMeta> core::LoadStaffTypeMetasFromFile(flecs::world& world, std::string_view file) {
    std::ifstream json_file{std::string(file)};

    if (!json_file.is_open()) {
        throw std::runtime_error(std::format("Could not open file: {}", file));
    }

    nlohmann::json json_data = nlohmann::json::parse(json_file);

    for (const auto& staff_set : json_data["staff_sets"]) {
        auto scope_s = staff_set["spritesheet_scope"].get<std::string>();
        auto name_s = staff_set["spritesheet_entity"].get<std::string>();
        flecs::entity spritesheet_e = world.lookup(std::format("{}::{}", scope_s, name_s).c_str());
        auto spritesheet_file_s = staff_set["spritesheet_path"].get<std::string>();

        if (!spritesheet_e) {
            auto texture = dxgl::LoadTextureFromFile(spritesheet_file_s);
            texture.SetFilterMode(dxgl::FilterMode::Nearest);

            spritesheet_e = world.entity(name_s.c_str())
                .set<dxgl::Texture>(std::move(texture));
        }
        
        dxgl::TextureView spritesheet_texture = spritesheet_e.get<dxgl::Texture>();

        for (const auto& meta_json : staff_set["staff"]) {
            StaffTypeMeta meta{};
            meta.type = meta_json["type"].get<std::string>();
            meta.display_name = meta_json["display_name"].get<std::string>();
            
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
}