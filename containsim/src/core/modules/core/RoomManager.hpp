#pragma once

#include <modules/core/Room.hpp>
#include <modules/core/Tile.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/application/EventManager.hpp>

#include <nano/nano_signal_slot.hpp>
#include <unordered_map>

namespace core {
    struct RoomCommand;
    
    class RoomManager {
        std::unordered_map<RoomId, Room> m_rooms{};
        TileGrid* m_tile_grid{};
        application::EventManager* m_event_manager{};
        RoomId m_next_id{1};

    public:
        RoomManager(TileGrid& tile_grid, application::EventManager& em);

        void MarkTilesAsRoom(const TileSelection& tiles, RoomType type);
        void UnmarkTiles(const TileSelection& tiles);

        auto* GetRoom(this auto&& self, RoomId room_id) noexcept {
            using Ret_t = decltype(&self.m_rooms.find(room_id)->second);

            if (room_id == NoRoom)
                return static_cast<Ret_t>(nullptr);

            auto it = self.m_rooms.find(room_id);

            if (it != self.m_rooms.end()) {
                return &it->second;
            } else {
                return static_cast<Ret_t>(nullptr);
            }
        }

        auto& GetRoomThrowing(this auto&& self, RoomId room_id) {
            return self.m_rooms.at(room_id);
        }

        const auto& GetRooms() const {
            return m_rooms;
        }

        const auto& GetTileGrid() const {
            return *m_tile_grid;
        }

    private:
        void ProcessCommand(const RoomCommand& cmd);
    };

    struct RoomCommand {
        std::function<void(RoomManager&)> execute;
    };
}