#include <dxgl/Vao.hpp>

#include <cassert>
#include <numeric>
#include <algorithm>
#include <format>
#include <ranges>
#include <stdexcept>
#include <glad/glad.h>
#include <xutility>

using namespace dxgl;

Vao::Vao() {
    glGenVertexArrays(1, &handle);
}

void Vao::UseImpl() const {
    glBindVertexArray(handle);
}

void Vao::DestroyImpl() const {
    glDeleteVertexArrays(1, &handle);
}

AttribGroup& AttribGroup::Attrib(const Attribute& a) {
    for (int i = 0; i < a.multiply; i++) {
        attributes.push_back(a);
    }

    return *this;
}

AttribGroup& AttribGroup::Vbo(VboView vbo) {
    this->vbo = vbo;
    return *this;
}


VaoAttribBuilder& VaoAttribBuilder::Group(AttribGroup g) {
    groups.push_back(std::move(g));
    return *this;
}

static std::size_t GetAttribTypeSize(AttribType type) {
    using enum AttribType;

    switch (type) {
        case Byte:
        case Ubyte:
            return 1;
        case Short:
        case Ushort:
            return 2;
        case Float:
        case Int:
        case Uint:
        case Fixed:
            return 4;
        case Double:
            return 8;
    }
}

static int GetAttribTypeGlEnum(AttribType type) {
    using enum AttribType;

    switch (type) {
        case Byte: return GL_BYTE;
        case Ubyte: return GL_UNSIGNED_BYTE;
        case Short: return GL_SHORT;
        case Ushort: return GL_UNSIGNED_SHORT;
        case Float: return GL_FLOAT;
        case Int: return GL_INT;
        case Uint: return GL_UNSIGNED_INT;
        case Fixed: return GL_FIXED;
        case Double: return GL_DOUBLE;
    }
}

void VaoAttribBuilder::Apply(VaoRef vao, VboView vbo_for_all) {
    namespace vws = std::views;
    using enum AttribType;

    vao->Use();

    std::ranges::sort(groups, {}, &AttribGroup::offset);

    // if (auto it = std::ranges::adjacent_find(groups, {}, &AttribGroup::offset); it != groups.end()) {
    //     throw std::runtime_error(
    //         std::format(
    //             "Multiple attribute groups with same offset ({}) detected", it->offset
    //         )
    //     );
    // }


    GLuint cur_loc = 0;
    
    for (const auto& group : groups) {
        if (group.vbo.HasValue()) { 
            group.vbo->Use();
        } else {
            assert(vbo_for_all.HasValue());
            vbo_for_all->Use();
        }

        auto attrib_sizes = vws::transform(group.attributes, [](const Attribute& a) {
            return a.components * GetAttribTypeSize(a.type) + a.padding;
        });

        auto stride = std::accumulate(attrib_sizes.begin(), attrib_sizes.end(), std::size_t{});
        std::size_t cur_offset = group.offset;
        auto attrib_size_it = attrib_sizes.begin();

        for (const auto& attrib : group.attributes) {
            switch (attrib.type) {
                case Float:
                case Double:
                case Fixed:
                    glVertexAttribPointer(
                        cur_loc,
                        attrib.components,
                        GetAttribTypeGlEnum(attrib.type),
                        GL_FALSE,
                        stride,
                        (void*) cur_offset
                    );
                    break;
                default: // All integer types need to go through the AttribI version
                    glVertexAttribIPointer(
                        cur_loc,
                        attrib.components,
                        GetAttribTypeGlEnum(attrib.type),
                        stride,
                        (void*) cur_offset
                    );
            }
            
            glVertexAttribDivisor(cur_loc, attrib.divisor);
            glEnableVertexAttribArray(cur_loc);

            cur_loc++;
            cur_offset += *attrib_size_it;
            attrib_size_it++;
        }
    }
}