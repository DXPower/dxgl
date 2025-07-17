#include <modules/misc_gameplay/MiscGameplay.hpp>
#include <modules/rendering/Rendering.hpp>
#include <modules/application/Application.hpp>
#include <modules/core/Core.hpp>
#include <modules/physics/Physics.hpp>

#include <modules/core/Transform.hpp>

#include <GLFW/glfw3.h>

#include <common/Logging.hpp>

using namespace misc_gameplay;
using namespace core;

namespace {
struct InputResults {
    glm::vec2 camera_movement{};
};

InputResults ProcessInput(GLFWwindow* window) {
    InputResults result{};

    auto IsKeyPressed = [window](int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    };

    if (IsKeyPressed(GLFW_KEY_W))
        result.camera_movement.y -= 1;
    if (IsKeyPressed(GLFW_KEY_S))
        result.camera_movement.y += 1;
    if (IsKeyPressed(GLFW_KEY_A))
        result.camera_movement.x -= 1;
    if (IsKeyPressed(GLFW_KEY_D))
        result.camera_movement.x += 1;

    if (result.camera_movement != glm::vec2(0, 0))
        result.camera_movement = glm::normalize(result.camera_movement);

    return result;
}

void CameraMovement(flecs::entity e, const dxgl::Window& window, rendering::Camera& camera) {
    constexpr float camera_speed = 350.f;
    
    auto input [[maybe_unused]] = ProcessInput(window.GetGlfwWindow());
    camera.MoveBy(input.camera_movement * camera_speed * e.world().delta_time());
}

void InitTilePrefabs(flecs::world& world) {
    const auto& tile_size = world.get<core::TileWorldSize>().value;

    auto prefab = world.prefab("cs:PrefabWall")
        .set(core::Transform{
            .size = tile_size
        })
        .set(physics::Collider{
            .is_fixed = true
        })
        .add<physics::SquareCollider>()
        .add<TileCoord>();

    auto l = logging::CreateLogger("MiscGameplay");
    l.set_level(spdlog::level::debug);
    l.debug("Created tile prefab: {}", prefab.path().c_str());
}

}

MiscGameplay::MiscGameplay(flecs::world& world) {
    world.import<application::Application>();
    world.import<rendering::Rendering>();
    world.import<core::Core>();
    world.import<physics::Physics>();

    world.system<const dxgl::Window, rendering::Camera>()
        .term_at(0).with<application::MainWindow>()
        .term_at<rendering::Camera>().singleton()
        .each(&CameraMovement);

    InitTilePrefabs(world);
}