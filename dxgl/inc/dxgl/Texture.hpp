#pragma once

#include "Handle.hpp"

#include <dxtl/cstring_view.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/vec2.hpp>
#include <memory>

namespace dxgl {
    enum class TextureFormat {
        R = 1,
        RG,
        RGB,
        RGBA
    };

    class TextureSource {
        struct Deleter {
            void operator()(unsigned char* ptr) const;
        };

        std::unique_ptr<unsigned char, Deleter> m_data{};
        glm::ivec2 size{};
        TextureFormat format{};

    public:
        TextureSource() = default;
        explicit TextureSource(dxtl::cstring_view file);

        unsigned char* GetData() {
            return m_data.get();
        }

        const unsigned char* GetData() const {
            return m_data.get();
        }

        const glm::ivec2& GetDims() const {
            return size;
        }

        TextureFormat GetFormat() const {
            return format;
        }
    };

    class Texture : public Handle<Texture, Usable::Yes, int> {
        glm::ivec2 size{};

    public:
        Texture();
        explicit Texture(const TextureSource& source);

        void Load(const TextureSource& source);
        const glm::ivec2& GetSize() const { return size; }
        
    protected:
        void UseImpl(int texture_unit) const;
        void DestroyImpl() const;

        friend class Handle;
    };

    using TextureView = HandleRef<Texture, false>;
    using TextureRef = HandleRef<Texture, true>;

    Texture LoadTextureFromFile(dxtl::cstring_view file);
}