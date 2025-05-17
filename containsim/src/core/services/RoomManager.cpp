#include <services/RoomManager.hpp>
#include <boost/container/small_vector.hpp>
#include <ranges>

using namespace services;

RoomManager::RoomManager(TileGrid& tile_grid) : m_tile_grid(&tile_grid) {
}

Room& RoomManager::MarkTilesAsRoom(const TileSelection& tiles, RoomType type) {
    // Find all tiles that are marked as the same type and merge them into a single room
    
    // Just guessing a good number here for a realistic case of merging
    constexpr static std::size_t estimated_merge_count = 10;
    boost::container::small_vector<Room*, estimated_merge_count> rooms_to_merge{};
    std::unordered_set<TileCoord> tiles_to_merge{};

    auto RoomNotMerging = [&](RoomId id) {
        return std::ranges::find(rooms_to_merge, id, &Room::GetId) == rooms_to_merge.end();
    };

    for (auto coord : tiles.Iterate()) {
        auto room_id = m_tile_grid->GetRoomAt(coord);

        // Not part of a room - assign it to the room(s) being created/expanded/merged
        if (room_id == NoRoom) {
            tiles_to_merge.insert(coord);
        } else if (room_id != NoRoom && RoomNotMerging(room_id)) {
            // Already part of a room - add it to the list of rooms to merge
            // if the types are the same
            auto& room = GetRoomThrowing(room_id);

            if (room.GetType() == type) {
                rooms_to_merge.push_back(&room);
            }
        }
    }

    // Handle the cases:
    // 1. One room to merge - add the new tiles to it
    // 2. No rooms to merge - create a new room
    // 3. Two or more rooms to merge - merge them into a new room
    if (rooms_to_merge.size() == 1) {
        rooms_to_merge[0]->AddTiles(std::move(tiles_to_merge));
        return *rooms_to_merge[0];
    } else {
        Room new_room(m_next_id++, type);

        new_room.AddTiles(std::move(tiles_to_merge));

        for (auto* room : rooms_to_merge) {
            new_room.AddTiles(room->ExtractTiles());
        }

        auto [it, ins] = m_rooms.emplace(new_room.GetId(), std::move(new_room));
        return it->second;
    }
}

void RoomManager::UnmarkTiles(const TileSelection&) {

}
