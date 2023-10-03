#include <dxgl/Framebuffer.hpp>
#include <dxgl/Renderbuffer.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Vao.hpp>

namespace dxgl {
    class Screenbuffer {
        Framebuffer framebuffer{};
        Texture color_buffer{};
        Renderbuffer depth_stencil_buffer{};
        
        mutable Program screen_program{};
        Vao screen_vao{};
        Vbo screen_vbo{};

    public:
        Screenbuffer();

        void ResizeToScreen();
        void Use() const;

        void Render() const;

        TextureRef GetTexture();
        TextureView GetTexture() const;

        static void Unuse();
    };
}