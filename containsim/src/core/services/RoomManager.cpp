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
        for (auto coord : tiles_to_merge) {
            m_tile_grid->SetRoomAt(coord, rooms_to_merge[0]->GetId());
        }

        rooms_to_merge[0]->AddTiles(std::move(tiles_to_merge));
        room_modified_signal.fire(RoomModified{.room = rooms_to_merge[0]});

        return *rooms_to_merge[0];
    } else {
        auto new_id = m_next_id++;
        auto [it, ins] = m_rooms.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(new_id),
            std::forward_as_tuple(new_id, type)
        );
        Room& new_room = it->second;
        
        for (auto coord : tiles_to_merge) {
            m_tile_grid->SetRoomAt(coord, new_id);
        }

        new_room.AddTiles(std::move(tiles_to_merge));

        for (auto* room : rooms_to_merge) {
            for (auto coord : room->GetTiles()) {
                m_tile_grid->SetRoomAt(coord, new_id);
            }

            new_room.AddTiles(room->ExtractTiles());
            
            auto remove_id = room->GetId();
            m_rooms.erase(remove_id);
            
            room_removed_signal.fire(RoomRemoved{.id = remove_id});
        }

        room_added_signal.fire(RoomAdded{.room = &new_room});
        return it->second;
    }
}

void RoomManager::UnmarkTiles(const TileSelection&) {

}
