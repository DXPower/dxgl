#include <common/DebugDraws.hpp>
#include <common/Rendering.hpp>
#include <dxgl/Vao.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Uniform.hpp>

using namespace dxgl;

namespace {
Program program{};
}

void DebugDraws::Init(UboBindingManager& ubos, DrawQueues& queues) {
    m_queues = &queues;

    program = ProgramBuilder()
        .Vert("shaders/debug.vert")
        .Frag("shaders/debug.frag")
        .Link();
    
    ubos.BindUniformLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        program, 
        "camera"
    );
}

void DebugDraws::MakeWorldDraw(std::span<const glm::vec2> points, glm::vec4 color, PrimType prim_type) {
    using namespace dxgl;
    
    Draw draw{};
    draw.program = program;
    draw.num_indices = (uint32_t) points.size();
    draw.prim_type = prim_type;
    draw.options.wireframe = true;
    draw.options.line_width = 4.f;

    draw.uniform_applicator = [color](ProgramRef program) {
        Uniform::Set(*program, "color", color);
    };

    draw.vbo_storage.emplace_back().Upload(points, BufferUsage::Static);

    VaoAttribBuilder()
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2))
        )
        .Apply(draw.vao_storage.emplace(), draw.vbo_storage.back());

    m_queues->QueueOwnedDraw(RenderLayer::Debug, std::move(draw));
}