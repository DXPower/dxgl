#include "Uniform.hpp"

#include <glad/glad.h>
#include <array>

using namespace Uniform;

static unsigned int GetUniform(const Program& program, const std::string& name) {
    return glGetUniformLocation(program.GetHandle(), name.c_str());
}

void detail::SetFloats(Program& program, const std::string& name, std::span<const float> floats) {
    auto loc = GetUniform(program, name);
    program.Use();

    auto funcs = std::to_array({glUniform1fv, glUniform2fv, glUniform3fv, glUniform4fv});
    funcs.at(floats.size() - 1)(loc, 1, floats.data());
}

void detail::SetInts(Program& program, const std::string& name, std::span<const int> ints) {
    auto loc = GetUniform(program, name);
    program.Use();

    auto funcs = std::to_array({glUniform1iv, glUniform2iv, glUniform3iv, glUniform4iv});
    funcs.at(ints.size() - 1)(loc, 1, ints.data());
}

void detail::SetMatrixF(Program& program, const std::string& name, int size, std::span<const float> floats) {
    auto loc = GetUniform(program, name);
    program.Use();

    auto funcs = std::to_array({glUniformMatrix2fv, glUniformMatrix3fv, glUniformMatrix4fv});
    funcs.at(size - 2)(loc, 1, GL_FALSE, floats.data());
}
