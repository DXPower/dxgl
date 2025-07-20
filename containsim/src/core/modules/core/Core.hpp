#include <flecs.h>

#include <modules/core/Actor.hpp>
#include <modules/core/BuildManager.hpp>
#include <modules/core/Configuration.hpp>
#include <modules/core/Cooldown.hpp>
#include <modules/core/Intent.hpp>
#include <modules/core/Interaction.hpp>
#include <modules/core/Mobility.hpp>
#include <modules/core/RoomManager.hpp>
#include <modules/core/Science.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/core/Transform.hpp>
#include <modules/core/Time.hpp>

namespace core {
    class Core {
    public:
        Core(flecs::world& world);
    };
}