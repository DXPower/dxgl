#pragma once

#include "Handle.hpp"
#include "Shader.hpp"
#include "boost/describe/modifiers.hpp"
#include "boost/mp11/algorithm.hpp"
#include "dxtl/cstring_view.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <cstring>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <optional>
#include <type_traits>
#include <vector>
#include <boost/describe/class.hpp>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>

namespace dxgl {
    namespace detail {
        namespace desc = boost::describe;

        struct FieldLoc {
            std::size_t base_alignment{};
            std::size_t offset{};
            std::size_t size{};
        };

        constexpr std::size_t NextMultiple(std::size_t x, std::size_t m) {
            if (x % m == 0) {
                return x;
            } else {
                return ((x / m) + 1) * m;
            }
        }

        template<typename T>
        constexpr bool is_gl_scalar = std::is_integral_v<T> || std::is_floating_point_v<T>;

        template<typename T>
        struct is_gl_vec : std::false_type { };

        template<typename V, glm::length_t N, glm::qualifier Q>
        struct is_gl_vec<glm::vec<N, V, Q>> : std::true_type { };

        template<typename T>
        struct is_gl_mat : std::false_type { };

        template<typename V, glm::length_t C, glm::length_t R, glm::qualifier Q>
        struct is_gl_mat<glm::mat<C, R, V, Q>> : std::true_type { };

        template<typename T>
        constexpr void GetFieldLocs(std::vector<FieldLoc>& locs, std::size_t& cur_offset) {
            namespace rgs = std::ranges;
            namespace vws = std::views;

            if constexpr (is_gl_scalar<T>) {
                locs.push_back(FieldLoc{
                    .base_alignment = 4,
                    .offset = NextMultiple(cur_offset, 4),
                    .size = 4
                });

                cur_offset = locs.back().offset + 4;
            } else if constexpr (is_gl_vec<T>::value) {
                static_assert(T::length() != 3, "vec3 not well implemented in std140");

                if constexpr (T::length() == 2) {
                    locs.push_back(FieldLoc{
                        .base_alignment = 8,
                        .offset = NextMultiple(cur_offset, 8),
                        .size = 8
                    });
                } else {
                    locs.push_back(FieldLoc{
                        .base_alignment = 16,
                        .offset = NextMultiple(cur_offset, 16),
                        .size = 16
                    });
                }

                cur_offset = locs.back().offset + locs.back().size;
            } else if constexpr (is_gl_mat<T>::value) {
                typename T::col_type cols[T::length()];
                GetFieldLocs<decltype(cols)>(locs, cur_offset);
            } else if constexpr (std::is_array_v<T>) {
                using A_t = std::remove_all_extents_t<T>;
                constexpr auto arr_size = sizeof(T) / sizeof(A_t);

                if constexpr (is_gl_scalar<A_t> || is_gl_vec<A_t>::value) {
                    for (std::size_t i = 0; i < arr_size; i++) {
                        GetFieldLocs<glm::vec4>(locs, cur_offset);
                    }
                } else if constexpr (is_gl_mat<A_t>::value) {
                    typename A_t::col_type cols[A_t::length() * arr_size];
                    GetFieldLocs<decltype(cols)>(locs, cur_offset);
                } else {
                    // Array of structs
                    for (std::size_t i = 0; i < arr_size; i++) {
                        GetFieldLocs<A_t>(locs, cur_offset);
                    }
                }
            } else {
                // Structs
                using Members = desc::describe_members<T, desc::mod_any_access>;

                std::vector<FieldLoc> struct_locs;
                std::size_t cur_offset_in_struct = 0;

                boost::mp11::mp_for_each<Members>([&](auto descriptor) {
                    using M_t = std::remove_cvref_t<decltype(std::declval<T>().*descriptor.pointer)>;

                    GetFieldLocs<M_t>(struct_locs, cur_offset_in_struct);
                });

                auto max_alignment = rgs::max(struct_locs, {}, &FieldLoc::base_alignment).base_alignment;
                max_alignment = NextMultiple(max_alignment, 16);

                cur_offset = NextMultiple(cur_offset, max_alignment);

                rgs::copy(struct_locs | vws::transform([&](FieldLoc f) {
                    f.offset += cur_offset;
                    return f;
                }), std::back_inserter(locs));

                cur_offset += NextMultiple(cur_offset_in_struct, max_alignment);
            }
        }
        
        template<typename T>
        void WriteObjFields(const T& obj, std::vector<FieldLoc>::const_iterator& cur_loc, std::span<std::byte> out) {
            if constexpr (is_gl_scalar<T>) {
                memcpy(&out[cur_loc->offset], &obj, sizeof(obj));
                cur_loc++;
            } else if constexpr (is_gl_vec<T>::value) {
                static_assert(T::length() != 3, "vec3 not well implemented in std140");

                memcpy(&out[cur_loc->offset], &obj, sizeof(obj));
                cur_loc++;
            } else if constexpr (is_gl_mat<T>::value) {
                for (glm::length_t i = 0; i < T::length(); i++) {
                    WriteObjFields(obj[i], cur_loc, out);
                }
            } else if constexpr (std::is_array_v<T>) {
                using A_t = std::remove_all_extents_t<T>;
                const auto arr_size = std::size(obj);

                if constexpr (is_gl_scalar<A_t> || is_gl_vec<A_t>::value) {
                    for (std::size_t i = 0; i < arr_size; i++) {
                        WriteObjFields(obj[i], cur_loc, out);
                    }
                } else if constexpr (is_gl_mat<A_t>::value) {
                    for (std::size_t i = 0; i < arr_size; i++) {
                        for (std::size_t c = 0; c < A_t::length(); c++) {
                            WriteObjFields(obj[i][c], cur_loc, out);
                        }
                    }
                } else {
                    // Array of structs
                    for (std::size_t i = 0; i < arr_size; i++) {
                        WriteObjFields(obj[i], cur_loc, out);
                    }
                }
            } else {
                using Members = desc::describe_members<T, desc::mod_any_access>;

                boost::mp11::mp_for_each<Members>([&](auto descriptor) {
                    const auto& field = obj.*descriptor.pointer;
                    WriteObjFields(field, cur_loc, out);
                });
            }
        }
    }

    struct BinObj {
        std::unique_ptr<std::byte[]> data{};
        std::size_t size{};
    };

    template<typename T>
    BinObj Std140([[maybe_unused]] const T& obj) {
        std::vector<detail::FieldLoc> field_locs;
        std::size_t cur_offset = 0;

        detail::GetFieldLocs<T>(field_locs, cur_offset);

        BinObj bin_obj = {
            .data = std::make_unique<std::byte[]>(cur_offset),
            .size = cur_offset
        };

        auto field_locs_iter = field_locs.cbegin();
        detail::WriteObjFields(obj, field_locs_iter, std::span(bin_obj.data.get(), cur_offset));

        return bin_obj;
    }

    class Ubo : public Handle<Ubo, Usable::Yes> {
        std::size_t buffer_size{};

    public:
        Ubo();

        void Upload(const BinObj& obj);

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using UboRef = HandleRef<Ubo, true>;
    using UboView = HandleRef<Ubo, false>;

    class UboBindingManager {
        std::vector<UboRef> bindings;

    public:
        UboBindingManager();

        void BindUboLocation(std::size_t i, UboRef ubo);
        void BindUniformLocation(std::size_t i, ProgramRef program, dxtl::cstring_view uniform_name) const;
        UboView GetUbo(std::size_t i) const;

        void UnbindLocation(std::size_t i);
    };
}