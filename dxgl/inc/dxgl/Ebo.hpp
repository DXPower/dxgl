#pragma once

#include <dxgl/Common.hpp>
#include <dxgl/Handle.hpp>

#include <span>

namespace dxgl {
    class Ebo : public Handle<Ebo, Usable::Yes> {
    public:
        Ebo();

        void Upload(std::span<const std::byte> data, BufferUsage usage);

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using EboRef = HandleRef<Ebo, true>;
    using EboView = HandleRef<Ebo, false>;
}