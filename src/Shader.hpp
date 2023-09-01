#pragma once

#include <string_view>
#include <functional>

#include "Handle.hpp"

enum class ShaderType {
    Vertex,
    Fragment
};

class Shader : public Handle<Shader, Usable::No> {
public:
    Shader() = default;
    Shader(ShaderType type, std::string_view source);

protected:
    void DestroyImpl() const;

    friend class Handle;
};

class Program : public Handle<Program, Usable::Yes> {
public:
    Program() = default;
    Program(std::initializer_list<std::reference_wrapper<Shader>> shaders);

protected:
    void UseImpl() const;
    void DestroyImpl() const;

    friend class Handle;
};

using ProgramRef = HandleRef<Program, true>;
using ProgramView = HandleRef<Program, false>;