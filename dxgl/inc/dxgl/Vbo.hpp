#pragma once

#include <dxgl/Handle.hpp>

#include <span>
#include <ranges>
#include <type_traits>
#include <xutility>

namespace dxgl {
    namespace rgs = std::ranges;

    class Vbo : public Handle<Vbo, Usable::Yes> {
        std::size_t buffer_size{};

    public:
        Vbo();

        
        void Upload(std::span<const std::byte> data);

        void Upload(const auto& obj) {
            static_assert(std::is_trivially_copyable_v<decltype(obj)>);

            return Upload(
                std::span<const std::byte, std::dynamic_extent>(
                    static_cast<std::byte*>(&obj), sizeof(obj)
                )
            );
        }

        template<rgs::contiguous_range R>
        requires (not std::is_same_v<rgs::range_value_t<R>, std::byte>)
        void Upload(const R& r) {
            using V = rgs::range_value_t<R>;
            static_assert(std::is_trivially_copyable_v<V>);
            
            return Upload(
                std::as_bytes(
                    std::span(
                        rgs::begin(r), rgs::size(r)
                    )
                )
            );
        }

    protected:
        void UseImpl() const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using VboRef = HandleRef<Vbo, true>;
    using VboView = HandleRef<Vbo, false>;
}