#pragma once

#include <common/Room.hpp>
#include <common/Tile.hpp>
#include <services/TileGrid.hpp>
#include <unordered_map>

#include <services/commands/CommandChains.hpp>
#include <services/commands/RoomCommands.hpp>

namespace services {
    class RoomManager : public commands::CommandConsumer<commands::RoomCommand> {
        std::unordered_map<RoomId, Room> m_rooms{};
        TileGrid* m_tile_grid{};
        RoomId m_next_id{1};

    public:
        RoomManager(TileGrid& tile_grid);

        Room& MarkTilesAsRoom(const TileSelection& tiles, RoomType type);
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

        void Consume(commands::RoomCommandPtr&& cmd) override {
            cmd->Execute(*this);
        }
    };
}