#include <flecs.h>

#include <modules/core/BuildManager.hpp>
#include <modules/core/Configuration.hpp>
#include <modules/core/RoomManager.hpp>
#include <modules/core/TileGrid.hpp>

namespace core {
    class Core {
    public:
        Core(flecs::world& world);
    };
}