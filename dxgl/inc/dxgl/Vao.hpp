#pragma once

#include "Handle.hpp"

#include <vector>

namespace dxgl {
    class Vao : public Handle<Vao, Usable::Yes> {
    public:
        Vao();

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using VaoRef = HandleRef<Vao>;

    enum class AttribType {
        Float, Byte, Ubyte, Short, Ushort, Int, Uint, Double, Fixed
    };

    struct Attribute {
        AttribType type{};
        int8_t components{};
        std::size_t padding{};

        Attribute& Type(AttribType t) { type = t; return *this; }
        Attribute& Components(int8_t c) { components = c; return *this; }
        Attribute& Padding(std::size_t p) { padding = p; return *this; }
    };

    struct AttribGroup {
        std::size_t offset{};
        std::vector<Attribute> attributes{};

        AttribGroup& Offset(std::size_t o) { offset = o; return *this; }
        AttribGroup& Attrib(const Attribute& a);
    };

    class VaoAttribBuilder {
        std::vector<AttribGroup> groups{};

    public:
        VaoAttribBuilder& Group(AttribGroup g);

        void Apply(VaoRef vao);
    };
}