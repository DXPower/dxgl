#pragma once

#include "Handle.hpp"

class Vao : public Handle<Vao, Usable::Yes> {
public:
    Vao();

protected:
    void UseImpl() const;
    void DestroyImpl() const;

    friend class Handle;
};

using VaoRef = HandleRef<Vao>;