#pragma once

#include <modules/core/Staff.hpp>
#include <common/ResourceRegistry.hpp>
#include <common/Logging.hpp>
#include <flecs.h>

namespace core {

class StaffManager {
    logging::Logger m_logger{logging::CreateLogger("StaffManager")};

    flecs::world* m_world{};
    const ResourceRegistry* m_resources{};

public:
    StaffManager(flecs::world& world, const ResourceRegistry& resources);

    flecs::entity CreateStaffMember(StaffType type);
};

}