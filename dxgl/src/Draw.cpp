#include <dxgl/Draw.hpp>

#include <cassert>
#include <glad/glad.h>

// static bool last_draw_wireframe = false;

void dxgl::Draw::Render() const {
    if (num_instances == 0 || num_indices == 0 || (uint64_t) first_index >= num_indices) {
        return; // Discard draw
    }

    program->Use();

    assert(vao_storage.has_value() ^ vao_view.HasValue());

    if (vao_storage.has_value()) {
        vao_storage->Use();
    } else {
        vao_view->Use();
    }

    if (uniform_applicator)
        uniform_applicator(program);

    for (int tex_unit = 0; const auto& tex : textures) {
        tex->Use(tex_unit++);
    }

    // if (last_draw_wireframe != (bool) options.wireframe) {
    //     last_draw_wireframe = (bool) options.wireframe;
    //     glPolygonMode(GL_FRONT_AND_BACK, options.wireframe ? GL_LINE : GL_FILL);
    // }

    if (prim_type == PrimType::Line || prim_type == PrimType::LineLoop || prim_type == PrimType::LineStrip) {
        glLineWidth(options.line_width);
    }

    glDrawArraysInstanced(static_cast<int>(prim_type), first_index, (int)num_indices, (int)num_instances);
}