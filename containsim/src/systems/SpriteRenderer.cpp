#include "common/GlobalState.hpp"
#include "dxgl/Common.hpp"
#include "dxgl/Draw.hpp"
#include "dxgl/Shader.hpp"
#include "dxgl/Texture.hpp"
#include "dxgl/Vao.hpp"
#include "dxgl/Vbo.hpp"
#include <dxgl//Uniform.hpp>
#include <memory>
#include <systems/SpriteRenderer.hpp>
#include <common/DrawQueues.hpp>

// #include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <array>

using namespace systems;

namespace {
    struct InstanceDataBuffer {
        glm::mat3 world{}; // center is origin
        glm::mat3 tex{}; // top left is origin
    };

    struct InstanceDataMisc {
        dxgl::TextureView spritesheet{};
    };

    struct InstanceData {
        std::vector<InstanceDataBuffer> instance_data_buffer{};
        std::vector<InstanceDataMisc> instance_data_misc{};
    };

    struct VertexData {
        glm::vec2 local_pos{};
        glm::vec2 tex_pos{};
    };

    constexpr std::array<VertexData, 4> quad_vbo_data{
               //  Pos            Tex
        VertexData{{-.5f, -.5f},  {0.f, 1.f}},
        VertexData{{ .5f, -.5f},  {1.f, 1.f}},
        VertexData{{ .5f,  .5f},  {1.f, 0.f}},
        VertexData{{-.5f,  .5f},  {0.f, 0.f}} 
    };
}

class SpriteRenderer::Pimpl {
public:
    std::map<RenderLayer, InstanceData> layer_instances{};

    dxgl::Program program{};
    dxgl::Vbo quad_vbo{};
};

SpriteRenderer::SpriteRenderer(const GlobalState& global_state, DrawQueues& queues) 
    : m_queues_out(&queues) {
    m_pimpl = std::make_unique<Pimpl>();

    m_pimpl->program = dxgl::ProgramBuilder()
        .Vert("shaders/sprite.vert")
        .Frag("shaders/sprite.frag")
        .Link();

    dxgl::Uniform::Set(m_pimpl->program, "spritesheet", 0);
    
    global_state.ubo_manager.BindUniformLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        m_pimpl->program, 
        "camera"
    );

    m_pimpl->quad_vbo.Upload(quad_vbo_data, dxgl::BufferUsage::Static);
}

SpriteRenderer::~SpriteRenderer() = default;


void SpriteRenderer::PreStore(
    const components::SpriteRenderer&,
    const components::Transform& transform,
    const components::RenderData& rdata, 
    const components::Sprite& sprite
) {
    auto world_mat = glm::mat3(1);
    world_mat = glm::translate(world_mat, transform.position);
    world_mat = glm::scale(world_mat, transform.size);

    glm::vec2 tex_size = sprite.spritesheet->GetSize();
    glm::vec2 pos_mod = { 
        sprite.cutout.position.x,
        tex_size.y - sprite.cutout.position.y - sprite.cutout.size.y
    };

    auto tex_mat = glm::mat3(1);
    tex_mat = glm::translate(tex_mat, pos_mod / tex_size);
    tex_mat = glm::scale(tex_mat, sprite.cutout.size / tex_size);

    // auto tex_proj = glm::ortho(0.f, (float) tex_size.x, (float) tex_size.y, 0.f, -1.f, 1.f);

    auto& instance_data = m_pimpl->layer_instances[rdata.layer];
    
    instance_data.instance_data_buffer.push_back({
        .world = world_mat,
        .tex = tex_mat
    });

    instance_data.instance_data_misc.emplace_back().spritesheet = sprite.spritesheet;
}

void SpriteRenderer::OnStore() {
    using namespace dxgl;

    for (const auto& [layer, instance_data] : m_pimpl->layer_instances) {
        auto draw = MakeDrawTemplate();

        draw.vbo_storage.emplace_back().Upload(instance_data.instance_data_buffer, BufferUsage::Static);
        draw.num_instances = instance_data.instance_data_buffer.size();
        // TODO: sort sprites by texture
        draw.textures.push_back(instance_data.instance_data_misc.front().spritesheet);

        VaoAttribBuilder()
            .Group(AttribGroup()
                .Vbo(m_pimpl->quad_vbo)
                .Attrib(Attribute()
                    .Type(AttribType::Float)
                    .Components(2)
                    .Multiply(2)
                )
            )
            .Group(AttribGroup()
                .Vbo(draw.vbo_storage.back())
                .Attrib(Attribute()
                    .Type(AttribType::Float)
                    .Matrix(3, 3)
                    .PerInstance()
                )
                .Attrib(Attribute()
                    .Type(AttribType::Float)
                    .Matrix(3, 3)
                    .PerInstance()
                )
                // .Offset(sizeof(quad_vbo_data))
            )
            .Apply(draw.vao);

        m_queues_out->m_draw_queues[layer].push_back(std::move(draw));
    }

    m_pimpl->layer_instances.clear();
}

dxgl::Draw SpriteRenderer::MakeDrawTemplate() const {
    dxgl::Draw draw{};

    draw.program = m_pimpl->program;
    
    draw.prim_type = dxgl::PrimType::TriangleFan;
    draw.num_indices = 4;

    return draw;
}