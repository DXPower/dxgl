#pragma once

#include "LightColor.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TextureStore.hpp"
#include "Vao.hpp"

#include <glm/glm.hpp>
#include <string_view>
#include <vector>
#include <array>

class Camera;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 tex_coords;
};

struct Triangle {
    std::array<Vertex, 3> vertices{};
};

struct TexColor {
    TextureRef ambient{};
    TextureRef diffuse{};
    TextureRef specular{};
};

struct MeshMaterial {
    LightColor color{};
    TexColor tex_color{};
    float shininess{};
};

class MeshPrimitive {
    std::vector<Triangle> triangles{};
    MeshMaterial material{};
    Vao vao{};
    
public:
    void SetTriangles(decltype(triangles) triangles);
    const decltype(triangles)& GetTriangles() const { return triangles; }

    void SetMaterial(const MeshMaterial& mat) { material = mat; }
    const MeshMaterial& GetMaterial() const { return material; }

    void Render(ProgramRef program) const;

private:
    void CreateVao();
};

struct Mesh {
    std::string name{};
    std::vector<MeshPrimitive> primitives{};
};

struct Model {
    std::vector<Mesh> meshes;
    mutable ProgramRef program;

    glm::vec3 position{};
    glm::vec3 scale{1, 1, 1};
    glm::vec3 rotation{};

    void Render(const Camera& camera) const;
};

Model LoadModelFromFile(std::string_view path, TextureStore& store);