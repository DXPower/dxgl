#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;
class Camera;

namespace detail {
    Camera MakeCamera();
}

class Camera {
    glm::vec3 camera_pos{};
    glm::vec3 camera_front{0.f, 0.f, -1.f};
    glm::vec3 camera_up{0.f, 1.f,  0.f};

    float camera_pitch{};
    float camera_yaw{-90.f};

    float fov = 45.f;

    glm::vec<2, int> window_size{};
    glm::mat4 projection_matrix{}; // Memoize because this won't change frequently

    Camera() = default;
public:
    static Camera& Get();

    const glm::vec3& GetPosition() const;
    const glm::vec3& GetDirection() const;
    
    void UpdatePosition(GLFWwindow* window, float delta_time);
    void UpdateWindowSize(int width, int height);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

private:
    void UpdateProjectionMatrix();

    friend Camera detail::MakeCamera();
};

