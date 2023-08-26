#pragma once

#include "Handle.hpp"

class VAO : public Handle<VAO, Usable::Yes> {
public:
    VAO();

protected:
    void UseImpl() const;
    void DestroyImpl() const;

    friend class Handle;
};