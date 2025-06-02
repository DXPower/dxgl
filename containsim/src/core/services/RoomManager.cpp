#include <services/RoomManager.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <limits>

using namespace services;
using boost::container::small_vector;

RoomManager::RoomManager(TileGrid& tile_grid) : m_tile_grid(&tile_grid) {
}

namespace {
// Just guessing a good number here for a realistic case of merging
constexpr std::size_t estimated_merge_count = 10;

struct MergeResults {
    struct Region {
        std::unordered_set<TileCoord> unmerged_tiles{};
        std::unordered_set<RoomId> rooms_to_merge{};
    };

    small_vector<Region, estimated_merge_count> regions;
};

void FloodfillDfs(
    const RoomManager& rooms,
    const TileGrid& grid,
    std::unordered_set<TileCoord>& working_set,
    const std::unordered_set<TileCoord>::iterator& start_it,
    RoomType type_to_merge,
    MergeResults::Region& region
) {
    const auto start_coord = *start_it;
    const auto room_id = grid.GetRoomAt(start_coord);
    auto start_handle = working_set.extract(start_it);

    if (room_id == NoRoom) {
        region.unmerged_tiles.insert(std::move(start_handle));
    } else if (rooms.GetRoom(room_id)->GetType() == type_to_merge) {
        region.rooms_to_merge.insert(room_id);
    } else {
        return; // Not the type we are looking for
    }

    // Check neighbors
    const auto neighbors = GetTileCoordNeighbors(grid, start_coord).ToList();

    for (const TileCoord& neighbor : neighbors) {
        auto neighbor_it = working_set.find(neighbor);

        if (neighbor_it != working_set.end()) {
            // If the neighbor is in the working set, continue the flood fill
            FloodfillDfs(rooms, grid, working_set, neighbor_it, type_to_merge, region);
        }
    }
}

MergeResults CalculateMerges(
    const RoomManager& rooms,
    const TileSelection& tiles,
    RoomType type_to_merge
) {
    const auto& grid = rooms.GetTileGrid();

    MergeResults results{};
    std::unordered_set<TileCoord> working_set{};

    // Initialize the working set with all tiles in the selection
    for (const TileCoord& coord : tiles.Iterate()) {
        working_set.insert(coord);
    }

    while (!working_set.empty()) {
        auto& region = results.regions.emplace_back();

        // Take a tile from the working set to start the flood fill
        auto start_it = working_set.begin();
        FloodfillDfs(rooms, grid, working_set, start_it, type_to_merge, region);
    }

    return results;
}
}

void RoomManager::MarkTilesAsRoom(const TileSelection& tiles, RoomType type) {
    auto merges = CalculateMerges(*this, tiles, type);

    for (auto& region : merges.regions) {
        // Two cases:
        // 1. No rooms to merge - create a new room with the unmerged tiles
        // 2. One or more rooms to merge - merge them into a new room, plus unmerged tiles
        if (region.rooms_to_merge.empty()) {
            // No rooms to merge, create a new room with the unmerged tiles
            auto new_id = m_next_id++;
            auto [it, ins] = m_rooms.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(new_id),
                std::forward_as_tuple(new_id, type)
            );

            Room& new_room = it->second;

            for (const TileCoord& coord : region.unmerged_tiles) {
                m_tile_grid->SetRoomAt(coord, new_id);
            }
            
            new_room.AddTiles(std::move(region.unmerged_tiles));

            room_added_signal.fire(RoomAdded{.room = &new_room});
        } else {
            // Merge existing rooms with the unmerged tiles
            auto new_id = m_next_id++;
            auto [it, ins] = m_rooms.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(new_id),
                std::forward_as_tuple(new_id, type)
            );
            Room& new_room = it->second;

            // Add unmerged tiles to the new room
            for (const TileCoord& coord : region.unmerged_tiles) {
                m_tile_grid->SetRoomAt(coord, new_id);
            }
            new_room.AddTiles(std::move(region.unmerged_tiles));

            // Merge existing rooms
            for (RoomId room_id : region.rooms_to_merge) {
                auto& room = GetRoomThrowing(room_id);

                for (const TileCoord& coord : room.GetTiles()) {
                    m_tile_grid->SetRoomAt(coord, new_id);
                }

                new_room.AddTiles(room.ExtractTiles());

                auto remove_id = room.GetId();
                m_rooms.erase(remove_id);
                
                room_removed_signal.fire(RoomRemoved{.id = remove_id});
            }

            room_added_signal.fire(RoomAdded{.room = &new_room});
        }
    }
}

void RoomManager::UnmarkTiles(const TileSelection&) {

}
