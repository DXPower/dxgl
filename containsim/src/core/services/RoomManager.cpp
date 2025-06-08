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

struct RegionResults {
    struct Region {
        std::unordered_set<TileCoord> unmerged_tiles{};
        std::unordered_set<RoomId> rooms_to_merge{};
    };

    small_vector<Region, estimated_merge_count> regions;
};

enum class FloodfillType : std::uint8_t {
    Merge,
    Split
};

template<FloodfillType Type>
void FloodfillDfs(
    const RoomManager& rooms,
    const TileGrid& grid,
    std::unordered_set<TileCoord>& working_set,
    const std::unordered_set<TileCoord>::iterator& start_it,
    RoomType type_to_merge,
    RegionResults::Region& region
) {
    const auto start_coord = *start_it;
    const auto room_id = grid.GetRoomAt(start_coord);
    auto start_handle = working_set.extract(start_it);

    if constexpr (Type == FloodfillType::Merge) {
        if (room_id == NoRoom) {
            region.unmerged_tiles.insert(std::move(start_handle));
        } else if (rooms.GetRoom(room_id)->GetType() == type_to_merge) {
            region.rooms_to_merge.insert(room_id);
        } else {
            return; // Not the type we are looking for
        }
    } else {
        region.unmerged_tiles.insert(std::move(start_handle));
    }

    // Check neighbors
    const auto neighbors = GetTileCoordNeighbors(grid, start_coord).ToList();

    for (const TileCoord& neighbor : neighbors) {
        auto neighbor_it = working_set.find(neighbor);

        if (neighbor_it != working_set.end()) {
            // If the neighbor is in the working set, continue the flood fill
            FloodfillDfs<Type>(rooms, grid, working_set, neighbor_it, type_to_merge, region);
        }
    }
}

RegionResults CalculateMerges(
    const RoomManager& rooms,
    const TileSelection& tiles,
    RoomType type_to_merge
) {
    const auto& grid = rooms.GetTileGrid();

    RegionResults results{};
    std::unordered_set<TileCoord> working_set{};

    // Initialize the working set with all tiles in the selection
    for (const TileCoord& coord : tiles.Iterate()) {
        working_set.insert(coord);
    }

    while (!working_set.empty()) {
        auto& region = results.regions.emplace_back();

        // Take a tile from the working set to start the flood fill
        auto start_it = working_set.begin();
        FloodfillDfs<FloodfillType::Merge>(rooms, grid, working_set, start_it, type_to_merge, region);
    }

    return results;
}

RegionResults CalculateSplits(
    const RoomManager& rooms,
    const TileSelection& tiles,
    std::unordered_set<RoomId>& visited_rooms
) {
    const auto& grid = rooms.GetTileGrid();

    RegionResults results{};
    for (const TileCoord& coord : tiles.Iterate()) {
        // If we have already visited this room, skip it.
        const auto room_id = grid.GetRoomAt(coord);
        if (room_id == NoRoom || visited_rooms.contains(room_id))
            continue;
        
        visited_rooms.insert(room_id);
        
        const auto& room = rooms.GetRoomThrowing(room_id);

        // Copy the room's tiles into the working set,
        // as long as they are not in the tiles we are removing.
        std::unordered_set<TileCoord> working_set{};
        const auto& room_tiles = room.GetTiles();
        std::ranges::copy_if(room_tiles, std::inserter(working_set, working_set.end()),
            [&tiles](const TileCoord& coord) {
                return !tiles.Contains(coord);
            }
        );

        // Now, we can flood fill the working set to find remaining regions after removing the tiles
        while (!working_set.empty()) {
            auto& region = results.regions.emplace_back();

            // Take a tile from the working set to start the flood fill
            const auto start_it = working_set.begin();
            FloodfillDfs<FloodfillType::Split>(rooms, grid, working_set, start_it, room.GetType(), region);
        }
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

void RoomManager::UnmarkTiles(const TileSelection& tiles) {
    std::unordered_set<RoomId> visited_rooms{};
    auto splits = CalculateSplits(*this, tiles, visited_rooms);

    // Clear the room in the tile selection
    for (const TileCoord& coord : tiles.Iterate()) {
        m_tile_grid->SetRoomAt(coord, NoRoom);
    }

    for (auto& region : splits.regions) {
        const auto& old_room_id = m_tile_grid->GetRoomAt(*region.unmerged_tiles.begin());
        const auto& old_room = GetRoomThrowing(old_room_id);
        const auto new_id = m_next_id++;

        // Create a new room with the remaining tiles
        auto [it, ins] = m_rooms.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(new_id),
            std::forward_as_tuple(new_id, old_room.GetType())
        );

        Room& new_room = it->second;

        for (const TileCoord& coord : region.unmerged_tiles) {
            m_tile_grid->SetRoomAt(coord, new_id);
        }

        new_room.AddTiles(std::move(region.unmerged_tiles));
        room_added_signal.fire(RoomAdded{.room = &new_room});
    }
    
    for (RoomId room_id : visited_rooms) {
        // Remove the room from the manager
        m_rooms.erase(room_id);
        room_removed_signal.fire(RoomRemoved{.id = room_id});
    }
}
