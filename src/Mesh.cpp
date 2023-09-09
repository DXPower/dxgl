#include "Mesh.hpp"
#include "Camera.hpp"
#include "Uniform.hpp"
#include "dxtl/cstring_view.hpp"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <span>

#include <tiny_obj_loader/tiny_obj_loader.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

void MeshPrimitive::SetTriangles(decltype(triangles) triangles) {
    this->triangles = std::move(triangles);
    CreateVao();
}

void MeshPrimitive::CreateVao() {
    vao.Use();

    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangles[0]) * triangles.size(), triangles.data(), GL_STATIC_DRAW);
    
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    // vertex texture coords
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

    glBindVertexArray(0);
}

namespace Uniform {
    void Set(Program& program, dxtl::cstring_view name, const MeshMaterial& material) {
        auto SetUniform = [&](const std::string& item_name, const auto& u) {
            Uniform::Set(program, std::string(name) + '.' + item_name, u);
        };
        
        SetUniform("color.ambient", material.color.ambient);
        SetUniform("color.diffuse", material.color.diffuse);
        SetUniform("color.specular", material.color.specular);

        // SetUniform("tex_color.ambient", 0);
        SetUniform("tex_color.diffuse_map", 0);
        SetUniform("tex_color.specular_map", 1);
        SetUniform("tex_color.emission_map", 2);
        // material.tex_color.ambient->Use(0);
        material.tex_color.diffuse->Use(0);
        material.tex_color.specular->Use(1);
        material.tex_color.emission->Use(2);
        
        SetUniform("shininess", material.shininess);
    }
}

void MeshPrimitive::Render(ProgramRef program) const {
    program->Use();
    vao.Use();

    Uniform::Set(*program, "material", material);

    glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);
}

void Model::Render(const Camera& camera) const {
    program->Use();

    auto model = glm::mat4(1);
    model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
    model = glm::translate(model, position);
    model = glm::scale(model, scale);

    auto view = camera.GetViewMatrix();
    auto projection = camera.GetProjectionMatrix();

    Uniform::Set(*program, "model", model);
    Uniform::Set(*program, "view", view);
    Uniform::Set(*program, "projection", projection);
    Uniform::Set(*program, "view_pos", camera.GetPosition());
    
    for (const auto& mesh : meshes) {
        for (const auto& mesh_prim : mesh.primitives) {
            mesh_prim.Render(program);
        }
    }
}

namespace fs = std::filesystem;
namespace to = tinyobj;

static glm::vec2 FormatVec2(std::span<const to::real_t, 2> data) {
    return glm::vec2{ data[0], data[1] };
}

static glm::vec3 FormatVec3(std::span<const to::real_t, 3> data) {
    return glm::vec3{ data[0], data[1], data[2] };
}

static Triangle FormatTriangle(
    const to::attrib_t& attribs,
    std::span<const to::index_t, 3> tri_indices
) {

    Triangle triangle{};

    for (std::size_t v = 0; v < 3; v++) {
        Vertex vert{};

        std::span<const to::real_t, 3> vert_xyz(&attribs.vertices[tri_indices[v].vertex_index * 3], 3);
        vert.position = FormatVec3(vert_xyz);

        std::span<const to::real_t, 3> norm_xyz(&attribs.normals[tri_indices[v].normal_index * 3], 3);
        vert.normal = FormatVec3(norm_xyz);

        std::span<const to::real_t, 2> tex_xy(&attribs.texcoords[tri_indices[v].texcoord_index * 2], 2);
        vert.tex_coords = FormatVec2(tex_xy);

        std::span<const to::real_t, 3> colors(&attribs.colors[tri_indices[v].vertex_index * 3], 3);
        vert.color = FormatVec3(colors);

        triangle.vertices[v] = vert;
    }

    return triangle;
}

static MeshMaterial FormatMaterialData(const to::material_t& mat, TextureStore& store) {
    MeshMaterial ret{
        .color{
            // .ambient = FormatVec3(mat.ambient),
            .diffuse = FormatVec3(mat.diffuse),
            .specular = FormatVec3(mat.specular)
        },
        .shininess = mat.shininess
    };

    TextureRef white = store.LoadTexture("res/img/white.png");
    TextureRef black = store.LoadTexture("res/img/black.png");

    if (!mat.diffuse_texname.empty())
        ret.tex_color.diffuse = store.LoadTexture(mat.diffuse_texname);
    else
        ret.tex_color.diffuse = white;

    // if (!mat.ambient_texname.empty())
        // ret.tex_color.ambient = store.LoadTexture(mat.ambient_texname);
    // else if (!mat.diffuse_texname.empty())
        // ret.tex_color.ambient = ret.tex_color.diffuse;
    // else
        // ret.tex_color.ambient = white;

    if (!mat.specular_texname.empty())
        ret.tex_color.specular = store.LoadTexture(mat.specular_texname);
    else
        ret.tex_color.specular = white;

    if (!mat.emissive_texname.empty())
        ret.tex_color.emission = store.LoadTexture(mat.emissive_texname);
    else
        ret.tex_color.emission = black;

    return ret;
}

Model LoadModelFromFile(std::string_view obj_path_str, TextureStore& store) {
    to::attrib_t attribs;
    std::vector<to::shape_t> shapes;
    std::vector<to::material_t> materials;
        
    std::string warn;
    std::string err;
    
    auto obj_path = fs::path(obj_path_str);
    auto mat_dir = obj_path.parent_path();

    bool ret = to::LoadObj(&attribs, &shapes, &materials,
        &warn, &err, obj_path.string().c_str(), mat_dir.string().c_str(), true);

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        throw std::runtime_error(std::format("Failed to load model {}", obj_path_str));
    }

    printf("# of vertices  = %d\n", (int)(attribs.vertices.size()) / 3);
    printf("# of normals   = %d\n", (int)(attribs.normals.size()) / 3);
    printf("# of texcoords = %d\n", (int)(attribs.texcoords.size()) / 2);
    printf("# of materials = %d\n", (int)materials.size());
    printf("# of shapes    = %d\n", (int)shapes.size());

    auto cwd = fs::current_path();
    fs::current_path(mat_dir);

    Model model;

    for (const auto& shape : shapes) {
        // Make one primitive per material used in this mesh
        std::vector<std::vector<Triangle>> mesh_prim_tris(materials.size());

        for (std::size_t tri = 0; tri < shape.mesh.indices.size(); tri += 3) {
            std::span<const to::index_t, 3> tri_indices(&shape.mesh.indices[tri], 3);

            Triangle triangle = FormatTriangle(attribs, tri_indices);

            int mat_idx = shape.mesh.material_ids[tri / 3];
            mesh_prim_tris[mat_idx].push_back(triangle);
        }

        Mesh mesh{};
        mesh.name = shape.name;

        // Only process the primitives that have triangles
        for (std::size_t mat_idx = 0; mat_idx < materials.size(); mat_idx++) {
            auto& tris = mesh_prim_tris[mat_idx];

            if (tris.empty())
                continue;

            MeshPrimitive mesh_prim;
            mesh_prim.SetTriangles(std::move(tris));
            mesh_prim.SetMaterial(FormatMaterialData(materials[mat_idx], store));

            mesh.primitives.push_back(std::move(mesh_prim));
        }

        if (!mesh.primitives.empty())
            model.meshes.push_back(std::move(mesh));
    }

    fs::current_path(cwd);

    return model;
}
