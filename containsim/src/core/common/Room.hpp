#pragma once

#include <common/TileCoord.hpp>
#include <span>
#include <unordered_set>
    #include <glm/gtx/hash.hpp>

enum class RoomType {
    ContainmentCell,
    Deliveries,
    Research,
    Security,
};

using RoomId = uint32_t;
inline constexpr RoomId NoRoom = 0;

class Room {
    std::unordered_set<TileCoord> m_tiles{};
    RoomId m_id{};
    RoomType m_type{};

public:
    Room(RoomId m_id, RoomType type);
    
    RoomType GetType() const { return m_type; }
    RoomId GetId() const { return m_id; }
    
    void AddTiles(std::span<const TileCoord> tiles);
    void AddTiles(std::unordered_set<TileCoord>&& tiles);
    bool ContainsTile(TileCoord tile) const;

    const std::unordered_set<TileCoord>& GetTiles() const { return m_tiles; }
    std::unordered_set<TileCoord> ExtractTiles();

    static Room Merge(RoomId new_id, Room&& a, Room&& b);
};