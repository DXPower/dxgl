#pragma once

#include "Handle.hpp"

#include <functional>
#include <string_view>
#include <optional>

namespace dxgl {
    enum class ShaderType {
        Vertex,
        Fragment,
        Geometry
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
        Program(std::initializer_list<std::reference_wrapper<const Shader>> shaders);

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using ProgramRef = HandleRef<Program, true>;
    using ProgramView = HandleRef<Program, false>;

    class ProgramBuilder {
        std::optional<Shader> frag{};
        std::optional<Shader> vert{};
        std::optional<Shader> geom{};

    public:
        ProgramBuilder& Frag(std::string_view file);
        ProgramBuilder& Vert(std::string_view file);
        ProgramBuilder& Geom(std::string_view file);

        Program Link() const;
    };
}