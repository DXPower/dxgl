#pragma once

#include <type_traits>

template<typename Self>
consteval bool ShouldAddConst() {
    return std::is_const_v<std::remove_reference_t<Self>>;
}

template<typename C, typename T>
using PropagateConst = std::conditional_t<ShouldAddConst<C>(), const T, T>;