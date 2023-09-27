#pragma once

#include "Handle.hpp"

namespace dxgl {
        
    class Framebuffer : public Handle<Framebuffer, Usable::Yes> {
    public:
        Framebuffer();

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using FramebufferRef = HandleRef<Framebuffer>;
}