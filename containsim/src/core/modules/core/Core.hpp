#include <flecs.h>

#include <modules/core/Configuration.hpp>
#include <modules/core/RoomManager.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/core/EventManager.hpp>

namespace core {
    class Core {
    public:
        Core(flecs::world& world);
    };
}