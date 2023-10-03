#include <dxgl/Graphics.hpp>
#include <glad/glad.h>

void dxgl::SetWireframe(bool enabled) {
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}