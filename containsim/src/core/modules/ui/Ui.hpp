#pragma once

#include <modules/ui/RmlEventManager.hpp>
#include <flecs.h>

namespace ui {

class Ui {
public:
    Ui(flecs::world& world);
};

}