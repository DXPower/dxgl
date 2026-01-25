#pragma once

namespace core {

using StaffType = int;

inline constexpr StaffType NoStaff = 0;

struct Staff {
    StaffType type{NoStaff};
};

}