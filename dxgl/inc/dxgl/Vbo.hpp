#pragma once

#include <dxgl/Common.hpp>
#include <dxgl/Handle.hpp>

#include <span>
#include <ranges>
#include <type_traits>

namespace dxgl {
    namespace rgs = std::ranges;

    class Vbo : public Handle<Vbo, Usable::Yes> {
        std::size_t buffer_size{};

    public:
        Vbo();

        
        void Upload(std::span<const std::byte> data, BufferUsage usage);

        template<typename T>
        void Upload(const T& obj, BufferUsage usage) {
            static_assert(std::is_trivially_copyable_v<T>);

            return Upload(
                std::span<const std::byte, std::dynamic_extent>(
                    reinterpret_cast<const std::byte*>(&obj), sizeof(obj)
                ),
                usage
            );
        }

        template<rgs::contiguous_range R>
        requires (not std::is_same_v<std::remove_const_t<rgs::range_value_t<R>>, std::byte>)
        void Upload(const R& r, BufferUsage usage) {
            using V = rgs::range_value_t<R>;
            static_assert(std::is_trivially_copyable_v<V>);
            
            return Upload(
                std::as_bytes(
                    std::span(
                        rgs::begin(r), rgs::size(r)
                    )
                ),
                usage
            );
        }

        void Update(std::span<const std::byte> data, std::size_t offset = 0);
        
        template<typename T>
        void Update(const T& obj, std::size_t offset = 0) {
            static_assert(std::is_trivially_copyable_v<T>);

            return Update(
                std::span<const std::byte, std::dynamic_extent>(
                    reinterpret_cast<const std::byte*>(&obj), sizeof(obj)
                ),
                offset
            );
        }

        template<rgs::contiguous_range R>
        requires (not std::is_same_v<std::remove_const_t<rgs::range_value_t<R>>, std::byte>)
        void Update(const R& r, std::size_t offset = 0) {
            using V = rgs::range_value_t<R>;
            static_assert(std::is_trivially_copyable_v<V>);
            
            return Update(
                std::as_bytes(
                    std::span(
                        rgs::begin(r), rgs::size(r)
                    )
                ),
                offset
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