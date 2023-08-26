#include "Camera.hpp"
#include <glfw/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

static Camera camera_singleton = detail::MakeCamera();

Camera detail::MakeCamera() {
    return Camera();
}

Camera& Camera::Get() {
    return camera_singleton;
}

void Camera::UpdatePosition(GLFWwindow* window, float delta_time) {
    const float camera_speed = 10.f * delta_time;
    const float rotation_speed = 75.f * delta_time;
    const float fov_speed = 35.f * delta_time;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_pos += camera_speed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_pos -= camera_speed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        camera_pitch += rotation_speed;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        camera_pitch -= rotation_speed;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        camera_yaw -= rotation_speed;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        camera_yaw += rotation_speed;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        fov -= fov_speed;
        UpdateProjectionMatrix();
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        fov += fov_speed;
        UpdateProjectionMatrix();
    }

    glm::vec3 direction;
    direction.x = cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch));
    direction.y = sin(glm::radians(camera_pitch));
    direction.z = sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch));
    camera_front = glm::normalize(direction);
}

void Camera::UpdateWindowSize(int width, int height) {
    window_size = { width, height };
    UpdateProjectionMatrix();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
}


glm::mat4 Camera::GetProjectionMatrix() const {
    return projection_matrix;
}

void Camera::UpdateProjectionMatrix() {
    projection_matrix = glm::perspective(
        glm::radians(fov),
        (float) window_size.x / (float) window_size.y,
        0.1f,
        100.f
    );
}