#pragma once

#include <dxgl/Shader.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Vao.hpp>

#include <boost/container/small_vector.hpp>

namespace dxgl {
    enum PrimType {
        Point = 0x0,
        Line,
        LineLoop,
        LineStrip,
        Triangle,
        TriangleStrip,
        TriangleFan,
        LineAdjacent = 0xA,
        LineStripAdjacent,
        TriangleAdjacent,
        TriangleStripAdjacent,
        Patch = 0xE
    };

    struct Draw {
        ProgramRef program{};
        Vao vao{};

        boost::container::small_vector<TextureView, 2> textures{};
        boost::container::small_vector<Vbo, 2> vbo_storage{};
        std::function<void(ProgramRef program)> uniform_applicator{};

        PrimType prim_type{};
        uint32_t num_indices{};
        int32_t first_index{};
        uint32_t num_instances = 1;

        void Render() const;
    };
}