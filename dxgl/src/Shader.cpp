#include <dxgl/Shader.hpp>

#include <fstream>
#include <format>
#include <glad/glad.h>
#include <initializer_list>
#include <stdexcept>

using namespace dxgl;

static std::string LoadFileToString(std::string_view path) {
    std::ifstream file{std::string(path), std::ios::binary};

    if (!file.is_open())
        throw std::runtime_error(std::format("Could not open file {}", path));


    std::string contents;

    file.seekg(0, std::ios::end);
    contents.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&contents[0], contents.size());

    return contents;
}

Shader::Shader(ShaderType type, std::string_view source) {
    auto gl_type = type == ShaderType::Vertex   ? GL_VERTEX_SHADER :
                   type == ShaderType::Fragment ? GL_FRAGMENT_SHADER :
                                                  GL_GEOMETRY_SHADER;

    std::string file_contents = LoadFileToString(source);
    const char* file_contents_arr[] = { file_contents.c_str() };

    handle = glCreateShader(gl_type);
    glShaderSource(handle, 1, file_contents_arr, nullptr);
    glCompileShader(handle);

    int success;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(handle, 512, nullptr, info_log);

        throw std::runtime_error(
            std::format("Shader compilation error.\n{}\n{}", source, info_log)
        );
    }
}

void Shader::DestroyImpl() const {
    glDeleteShader(handle);
}

Program::Program(std::initializer_list<std::reference_wrapper<Shader>> shaders) {
    handle = glCreateProgram();

    for (const auto& shader : shaders) {
        glAttachShader(handle, shader.get().GetHandle());
    }

    glLinkProgram(handle);

    int success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);

    if(!success) {
        char info_log[512];
        glGetProgramInfoLog(handle, 512, nullptr, info_log);
        
        throw std::runtime_error(
            std::format("Program linking error.\n{}", info_log)
        );
    }
}

void Program::UseImpl() const {
    glUseProgram(handle);
}

void Program::DestroyImpl() const{
    glDeleteProgram(handle);
}