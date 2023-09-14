#pragma once

#include "Handle.hpp"
#include "Texture.hpp"
#include "dxtl/cstring_view.hpp"

#include <array>
#include <ranges>
#include <span>
#include <type_traits>

class TextureSource;

class Cubemap : public Handle<Cubemap, Usable::Yes> {
public:
    Cubemap();
    Cubemap(std::span<const TextureSource, 6> faces);

    void Load(std::span<const TextureSource, 6> faces);

private:
    void UseImpl() const;
    void DestroyImpl() const;

    friend class Handle;
};

using CubemapRef = HandleRef<Cubemap>;

template<std::ranges::range R>
requires std::is_constructible_v<dxtl::cstring_view, std::ranges::range_value_t<R>>
Cubemap LoadCubemapFromFiles(R&& paths) {
    assert(std::ranges::size(paths) == 6);

    std::array<TextureSource, 6> paths_arr{};
    std::ranges::copy(paths | std::views::transform([](const dxtl::cstring_view& s) {
        return TextureSource(s);
    }), paths_arr.begin());

    return Cubemap(paths_arr);
}