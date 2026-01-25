#pragma once

#include <modules/core/Staff.hpp>
#include <modules/core/StaffTypeMeta.hpp>
#include <common/Logging.hpp>
#include <flecs.h>

namespace core {

class StaffManager {
    logging::Logger m_logger{logging::CreateLogger("StaffManager")};

    flecs::world* m_world{};
    const StaffTypeMetas* m_metas{};

public:
    StaffManager(flecs::world& world);

    flecs::entity CreateStaffMember(StaffType type);
};

}