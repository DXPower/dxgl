#include <dxgl/Application.hpp>
#include "dxgl/Framebuffer.hpp"
#include <dxgl/Graphics.hpp>
#include <dxgl/Screenbuffer.hpp>
#include <dxgl/Uniform.hpp>
#include <glad/glad.h>

using namespace dxgl;

Screenbuffer::Screenbuffer() {
    constexpr float vert_data[] = {
        -1, 1, 0, 1,  // Top left
        -1, -1, 0, 0, // Bottom left
        1, -1, 1, 0,  // Bottom right
        1, 1, 1, 1,   // Top right
    };

    screen_vao.Use();
    screen_vbo.Upload(vert_data);

    VaoAttribBuilder()
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
            )
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
            )
        )
        .Apply(screen_vao, screen_vbo);

    screen_program = ProgramBuilder()
        .Vert("shaders/framebuffer.vert")
        .Frag("shaders/framebuffer.frag")
        .Link();
        
    Uniform::Set(screen_program, "texture", 0);
}

void Screenbuffer::ResizeToScreen() {
    framebuffer.Use();
    color_buffer.Use(0);

    auto size = Application::GetWindowSize();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer.GetHandle(), 0);

    depth_stencil_buffer.Use();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_buffer.GetHandle());

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    throw std::runtime_error("Framebuffer is not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void Screenbuffer::Use() const {
    framebuffer.Use();
}

void Screenbuffer::Render() const {
    Unuse();
    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    screen_program.Use();
    screen_vao.Use();
    color_buffer.Use(0);
    
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

TextureRef Screenbuffer::GetTexture() {
    return color_buffer;
}

TextureView Screenbuffer::GetTexture() const {
    return color_buffer;
}

void Screenbuffer::Unuse() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}