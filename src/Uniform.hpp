#pragma once

#include "Shader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <span>

namespace Uniform {
    template<typename T, glm::length_t N>
    using Vec = glm::vec<N, T>;

    template<typename T, glm::length_t C, glm::length_t R = C>
    using Mat = glm::mat<C, R, T>;

    namespace detail {
        void SetFloats(Program& program, const std::string& name, std::span<const float> floats);
        void SetInts(Program& program, const std::string& name, std::span<const int> ints);
        void SetMatrixF(Program& program, const std::string& name, int size, std::span<const float> floats);
    }

    template<typename T, glm::length_t N>
    void Set(Program& program, const std::string& name, const Vec<T, N>& uniform) {
        using namespace detail;
        
        if constexpr (std::is_same_v<T, float>) {
            SetFloats(program, name, std::span(glm::value_ptr(uniform), N));
        } else if constexpr (std::is_same_v<T, int>) {
            SetInts(program, name, std::span(glm::value_ptr(uniform), N));
        }
    }

    template<glm::length_t C, glm::length_t R>
    void Set(Program& program, const std::string& name, const Mat<float, C, R>& uniform) {
        static_assert(C == R);
        detail::SetMatrixF(program, name, C, std::span(glm::value_ptr(uniform), C * R));
    }
}



