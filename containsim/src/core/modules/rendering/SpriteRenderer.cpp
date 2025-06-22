#include <dxgl/Common.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Vao.hpp>
#include <dxgl/Vbo.hpp>
#include <dxgl//Uniform.hpp>
#include <dxgl/Ubo.hpp>
#include <modules/rendering/SpriteRenderer.hpp>
#include <modules/rendering/Sprite.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/RenderData.hpp>
#include <components/Transform.hpp>

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <array>
#include <map>

using namespace rendering;

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

    struct SpriteRendererSystemData {
        std::map<RenderLayer, InstanceData> layer_instances{};

        dxgl::Program program{};
        dxgl::Vbo quad_vbo{};
        

        dxgl::Draw MakeDrawTemplate() {
            dxgl::Draw draw{};

            draw.program = program;
            
            draw.prim_type = dxgl::PrimType::TriangleFan;
            draw.num_indices = 4;

            return draw;
        }
    };

    void PreStore(
        const SpriteRenderer&,
        const components::Transform& transform,
        const RenderData& rdata, 
        const Sprite& sprite,
        SpriteRendererSystemData& sr_data
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

        auto& instance_data = sr_data.layer_instances[rdata.layer];
        
        instance_data.instance_data_buffer.push_back({
            .world = world_mat,
            .tex = tex_mat
        });

        instance_data.instance_data_misc.emplace_back().spritesheet = sprite.spritesheet;
    }

    void OnStore(SpriteRendererSystemData& sr_data, DrawQueues& draw_queues) {
        using namespace dxgl;

        for (const auto& [layer, instance_data] : sr_data.layer_instances) {
            auto draw = sr_data.MakeDrawTemplate();

            draw.vao_storage.emplace();
            draw.vbo_storage.emplace_back().Upload(instance_data.instance_data_buffer, BufferUsage::Static);
            draw.num_instances = (uint32_t) instance_data.instance_data_buffer.size();
            // TODO: sort sprites by texture
            draw.textures.push_back(instance_data.instance_data_misc.front().spritesheet);

            VaoAttribBuilder()
                .Group(AttribGroup()
                    .Vbo(sr_data.quad_vbo)
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
                .Apply(*draw.vao_storage);

            draw_queues.QueueOwnedDraw(layer, std::move(draw));
        }

        sr_data.layer_instances.clear();
    }
}

void rendering::SpriteRendererSystems(flecs::world& world) {
    auto& ubos = world.get_mut<dxgl::UboBindingManager>();

    world.component<SpriteRendererSystemData>();
    auto& sr_data = world.ensure<SpriteRendererSystemData>();

    sr_data.program = dxgl::ProgramBuilder()
        .Vert("shaders/sprite.vert")
        .Frag("shaders/sprite.frag")
        .Link();

    dxgl::Uniform::Set(sr_data.program, "spritesheet", 0);
    
    ubos.BindUniformLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        sr_data.program, 
        "camera"
    );

    sr_data.quad_vbo.Upload(quad_vbo_data, dxgl::BufferUsage::Static);

    // Register systems
    using components::Transform;
    world.system<const SpriteRenderer, const Transform, const RenderData, const Sprite, SpriteRendererSystemData>("SpriteRendererPreStore")
        .term_at<SpriteRendererSystemData>().singleton()
        .kind(flecs::PreStore)
        .each(&PreStore);

    world.system<SpriteRendererSystemData, DrawQueues>("SpriteRendererOnStore")
        .term_at<SpriteRendererSystemData>().singleton()
        .term_at<DrawQueues>().singleton()
        .kind(flecs::OnStore)
        .each(&OnStore);
}