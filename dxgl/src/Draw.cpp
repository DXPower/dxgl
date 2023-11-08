#include <dxgl/Draw.hpp>

#include <glad/glad.h>

void dxgl::Draw::Render() const {
    if (num_instances == 0 || num_indices == 0 || (uint64_t) first_index >= num_indices) {
        return; // Discard draw
    }

    program->Use();
    vao.Use();

    if (uniform_applicator)
        uniform_applicator(program);

    for (int tex_unit = 0; const auto& tex : textures) {
        tex->Use(tex_unit++);
    }

    glDrawArraysInstanced(static_cast<int>(prim_type), first_index, num_indices, num_instances);
}