#pragma once

#include "Handle.hpp"
#include <string_view>
#include <memory>

struct TexSize {
    int w{};
    int h{};
};

enum class TextureFormat {
    RGB,
    RGBA
};

class TextureSource {
    struct Deleter {
        void operator()(unsigned char* ptr);
    };

    std::unique_ptr<unsigned char, Deleter> m_data{};
    TexSize size{};
    TextureFormat format{};

public:
    TextureSource() = default;
    explicit TextureSource(std::string_view file);

    unsigned char* GetData() {
        return m_data.get();
    }

    const unsigned char* GetData() const {
        return m_data.get();
    }

    const TexSize& GetDims() const {
        return size;
    }

    TextureFormat GetFormat() const {
        return format;
    }
};

class Texture : public Handle<Texture, Usable::Yes, int> {
public:
    Texture();
    explicit Texture(const TextureSource& source);

    void Load(const TextureSource& source);
    
protected:
    void UseImpl(int texture_unit) const;
    void DestroyImpl() const;

    friend class Handle;
};

using TextureRef = HandleRef<Texture>;

Texture LoadTextureFromFile(std::string_view file);