#pragma once

#include "Handle.hpp"

namespace dxgl {
    class Renderbuffer : public Handle<Renderbuffer, Usable::Yes> {
    public:
        Renderbuffer();

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using RenderbufferRef = HandleRef<Renderbuffer>;
}