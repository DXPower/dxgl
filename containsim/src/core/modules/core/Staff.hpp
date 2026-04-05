#pragma once

#include <common/ResourceId.hpp>

namespace core {

using StaffType = ResourceId;

inline constexpr StaffType NoStaff = StaffType::Null;

struct Staff {
    StaffType type{NoStaff};
};

}