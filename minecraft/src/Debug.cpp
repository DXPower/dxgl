#include "Debug.hpp"
#include "GlobalState.hpp"

#include <dxgl/Vao.hpp>
#include <dxgl/Ebo.hpp>
#include <dxgl/Shader.hpp>

#include <span>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glad/glad.h>

namespace {
    struct DebugVert {
        glm::vec2 position{};
        glm::vec4 color{};
    };
}

class DebugDraws::Pimpl {
public:
    static constexpr uint16_t reset_index = 0xFFFF;

    std::vector<DebugVert> vertices{};
    std::vector<uint16_t> indices{};

    dxgl::Vao vao{};
    dxgl::Vbo vbo{};
    dxgl::Ebo ebo{};
    dxgl::Program program{};

    Pimpl(const GlobalState& global_state) {
        using namespace dxgl;

        VaoAttribBuilder()
            .Group(AttribGroup()
                .Attrib(Attribute()
                    .Components(2)
                    .Type(AttribType::Float)
                )
                .Attrib(Attribute()
                    .Components(4)
                    .Type(AttribType::Float)
                )
            )
            .Apply(vao, vbo);

        vao.Use();
        ebo.Use();

        program = ProgramBuilder()
            .Vert("shaders/debug.vert")
            .Frag("shaders/debug.frag")
            .Link();

        global_state.ubo_manager.BindUniformLocation(0, program, "camera");
    }

    void FinishPrim() {
        indices.push_back(reset_index);
    }

    void Reset() {
        vertices.clear();
        indices.clear();
    }
};

DebugDraws::DebugDraws() = default;
DebugDraws::~DebugDraws() = default;



void DebugDraws::Init(const GlobalState& global_state) {
    pimpl = std::make_unique<Pimpl>(global_state);
}

void DebugDraws::Draw(const DebugSquare& square, const glm::vec4& color) {
    const auto half = square.size / 2.f;
    const uint16_t base_idx = pimpl->vertices.size();

    pimpl->vertices.push_back({square.position + (half * glm::vec2(-1, -1)), color}); // Top left
    pimpl->vertices.push_back({square.position + (half * glm::vec2(1, -1)), color}); // Top right
    pimpl->vertices.push_back({square.position + (half * glm::vec2(1, 1)), color}); // Bottom right
    pimpl->vertices.push_back({square.position + (half * glm::vec2(-1, 1)), color}); // Bottom left

    pimpl->indices.push_back(base_idx);
    pimpl->indices.push_back(base_idx + 1);
    pimpl->indices.push_back(base_idx + 2);
    pimpl->indices.push_back(base_idx + 3);
    pimpl->indices.push_back(base_idx);

    pimpl->FinishPrim();
}

void DebugDraws::Draw(const DebugLine& line, const glm::vec4& color) {
    const uint16_t base_idx = pimpl->vertices.size();
    
    pimpl->vertices.push_back({line.from, color});
    pimpl->vertices.push_back({line.to, color});

    pimpl->indices.push_back(base_idx);
    pimpl->indices.push_back(base_idx + 1);

    pimpl->FinishPrim();
}

void DebugDraws::Draw(const DebugArrow& arrow, const glm::vec4& color) {
    const uint16_t base_idx = pimpl->vertices.size();
    
    const auto arrow_base = glm::lerp(arrow.from, arrow.to, 0.93f);
    const auto arrow_dir = glm::normalize(arrow.to - arrow.from);
    const auto rotated = glm::vec2(arrow_dir.y, -arrow_dir.x);
    const auto secondary_length = glm::max(glm::length(arrow.to - arrow.from) * 0.07f, 10.f);

    pimpl->vertices.push_back({arrow.from, color});
    pimpl->vertices.push_back({arrow.to, color});
    pimpl->vertices.push_back({arrow_base + (rotated * secondary_length), color});
    pimpl->vertices.push_back({arrow_base - (rotated * secondary_length), color});

    // Main line
    pimpl->indices.push_back(base_idx);
    pimpl->indices.push_back(base_idx + 1);
    pimpl->FinishPrim();

    // Arrow half
    pimpl->indices.push_back(base_idx + 1);
    pimpl->indices.push_back(base_idx + 2);
    pimpl->FinishPrim();

    // Arrow half
    pimpl->indices.push_back(base_idx + 1);
    pimpl->indices.push_back(base_idx + 3);
    pimpl->FinishPrim();
}

void DebugDraws::Render() const {
    pimpl->vao.Use();
    pimpl->program.Use();
    
    pimpl->vbo.Upload(pimpl->vertices, dxgl::BufferUsage::Dynamic);
    pimpl->ebo.Upload(std::as_bytes(std::span(pimpl->indices)), dxgl::BufferUsage::Dynamic);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glLineWidth(2);
    glDrawElements(GL_LINE_STRIP, pimpl->indices.size(), GL_UNSIGNED_SHORT, 0);
    glDisable(GL_PRIMITIVE_RESTART);

    // Clear all debug draws from the previous frame
    pimpl->Reset();
}