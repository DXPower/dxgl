#include <modules/core/RoomTypeMeta.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

using namespace core;

RoomType RoomTypeMetas::Add(RoomTypeMeta meta) {
    int new_id = m_ids.AddId(meta.type);
    m_metas.try_emplace(new_id, std::move(meta));
    return new_id;
}

const RoomTypeMeta* RoomTypeMetas::Get(RoomType id) const {
    auto it = m_metas.find(id);

    if (it == m_metas.end())
        return nullptr;

    return &it->second;
}

const RoomTypeMeta* RoomTypeMetas::Get(std::string_view type) const {
    auto id = GetIdOfType(type);

    if (id.has_value())
        return Get(*id);
    else
        return nullptr;
}

std::optional<RoomType> RoomTypeMetas::GetIdOfType(std::string_view type) const {
    const auto& id_map = m_ids.GetIds();

    auto it = id_map.right.find(std::string(type));

    if (it == id_map.right.end())
        return std::nullopt;

    return it->second;
}

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