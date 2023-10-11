#include "Player.hpp"

#include <dxgl/Uniform.hpp>
#include <dxgl/Shader.hpp>
#include <dxgl/Texture.hpp>
#include <dxgl/Vao.hpp>

#include <glad/glad.h>

namespace {
    struct PlayerDrawInfo {
        dxgl::Program program{};
        dxgl::Vao vao{};
        dxgl::Vbo vbo{};
        dxgl::Texture spritesheet{};
    };

    struct PlayerVbo {
        glm::vec2 position{};
        glm::vec2 sprite_origin{};
        glm::vec4 color{};
    };

    std::optional<PlayerDrawInfo> player_draw{};
}

Player::Player() {
    size = glm::vec2(100, 225);
}

void Player::Update(float delta_time) {
    MoveBy(velocity * delta_time);
}

void Player::Render() const {
    player_draw->program.Use();
    player_draw->vao.Use();
    player_draw->spritesheet.Use(0);

    player_draw->vbo.Update(position);
    dxgl::Uniform::Set(player_draw->program, "half_world_size", size / 2.f);

    glDrawArrays(GL_POINTS, 0, 1);
}

void Player::InitDraw(const GlobalState& global_state) {
    using namespace dxgl;

    auto program = ProgramBuilder()
        .Vert("shaders/sprite.vert")
        .Geom("shaders/sprite.geom")
        .Frag("shaders/sprite.frag")
        .Link();

    PlayerVbo player_vbo{};
    player_vbo.sprite_origin = {0, 1};
    
    Vbo vbo{};
    vbo.Upload(player_vbo, BufferUsage::Dynamic);

    Vao vao{};
    VaoAttribBuilder()
        .Group(AttribGroup()
            .Attrib(Attribute()
                .Type(AttribType::Float)
                .Components(2)
            )
        )
        .Apply(vao, vbo);


    auto spritesheet = LoadTextureFromFile("res/img/steve.png");
    
    Uniform::Set(program, "sprite_size", glm::vec2(112, 224) / (glm::vec2) spritesheet.GetSize());
    Uniform::Set(program, "spritesheet", 0);

    global_state.ubo_manager.BindUniformLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        program, 
        "camera"
    );

    player_draw = PlayerDrawInfo{
        std::move(program),
        std::move(vao),
        std::move(vbo),
        std::move(spritesheet)
    };
}
