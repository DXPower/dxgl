#include <dxgl/Ubo.hpp>
#include <dxgl/Shader.hpp>

#include <glad/glad.h>
#include <optional>

using namespace dxgl;

Ubo::Ubo() {
    glGenBuffers(1, &handle);
}

void Ubo::Upload(const BinObj& obj) {
    Use();
    
    if (buffer_size != obj.size) {
        glBufferData(GL_UNIFORM_BUFFER, obj.size, obj.data.get(), GL_STATIC_DRAW);
        buffer_size = obj.size;
    } else {
        glBufferSubData(GL_UNIFORM_BUFFER, 0, obj.size, obj.data.get());
    }
}

void Ubo::UseImpl() const {
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
}

void Ubo::DestroyImpl() const {
    glDeleteBuffers(1, &handle);
}

UboBindingManager::UboBindingManager() {
    int max_vert_uniform_bindings;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_vert_uniform_bindings);

    bindings.resize(max_vert_uniform_bindings);
}

void UboBindingManager::BindUboLocation(std::size_t i, UboRef ubo) {
    bindings.at(i) = ubo;
    glBindBufferBase(GL_UNIFORM_BUFFER, i, ubo->GetHandle());
}

void UboBindingManager::BindUniformLocation(std::size_t i, ProgramRef program, dxtl::cstring_view uniform_name) const {
    assert(i < bindings.size());
    auto uniform_idx = glGetUniformBlockIndex(program->GetHandle(), uniform_name.c_str());
    glUniformBlockBinding(program->GetHandle(), uniform_idx, i);
}

void UboBindingManager::UnbindLocation(std::size_t i) {
    bindings.at(i) = std::nullopt;
}