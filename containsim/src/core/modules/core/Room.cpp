#include <modules/core/Room.hpp>
#include <modules/core/Tile.hpp>

#include <format>
#include <stdexcept>
#include <magic_enum/magic_enum.hpp>

using namespace core;

Room::Room(RoomId id, RoomType type) : m_id(id), m_type(type) {}

void Room::AddTiles(std::span<const TileCoord> tiles) {
    for (const auto& tile : tiles) {
        m_tiles.insert(tile);
    }
}

void Room::AddTiles(std::unordered_set<TileCoord>&& tiles) {
    m_tiles.merge(tiles);
}

std::unordered_set<TileCoord> Room::ExtractTiles() {
    std::unordered_set<TileCoord> tiles{};
    m_tiles.swap(tiles);
    return tiles;
}

bool Room::ContainsTile(TileCoord tile) const {
    return m_tiles.contains(tile);
}

Room Room::Merge(RoomId new_id, Room&& a, Room&& b) {
    if (a.m_type != b.m_type) {
        throw std::invalid_argument(std::format("Cannot merge rooms of different types: {} and {}",
            magic_enum::enum_name(a.m_type), magic_enum::enum_name(b.m_type)));
    }

    Room merged(new_id, a.m_type);
    merged.m_tiles = std::move(a.m_tiles);
    merged.m_tiles.merge(b.m_tiles);

    return merged;
}