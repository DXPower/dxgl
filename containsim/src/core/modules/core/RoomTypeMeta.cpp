#include <modules/core/RoomTypeMeta.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

using namespace core;

std::generator<RoomTypeMeta> core::LoadRoomTypeMetasFromFile(std::string_view file) {
    std::ifstream json_file{std::string(file)};

    if (!json_file.is_open()) {
        throw std::runtime_error(std::format("Could not open file: {}", file));
    }

    nlohmann::json json_data = nlohmann::json::parse(json_file);

    for (const auto& meta_json : json_data["room_types"]) {
        RoomTypeMeta meta{};
        meta.type = meta_json["type"].get<std::string>();
        meta.display_name = meta_json["display_name"].get<std::string>();
        
        uint32_t color_int = meta_json["color"].get<uint32_t>();
        meta.color = glm::vec3(
            ((color_int >> 16) & 0xFF) / 255.f,
            ((color_int >> 8) & 0xFF) / 255.f,
            ((color_int >> 0) & 0xFF) / 255.f
        );

        co_yield std::move(meta);
    }
}