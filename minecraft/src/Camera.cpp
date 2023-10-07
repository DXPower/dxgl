#include "Camera.hpp"

#include <boost/describe/class.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace {
    struct CameraUbo {
        glm::mat4 camera_matrix;
    };

    BOOST_DESCRIBE_STRUCT(CameraUbo, (), (camera_matrix));
};

Camera::Camera(GlobalState& global_state) {
    UpdateView();

    global_state.ubo_manager.BindUboLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        ubo
    );
}

void Camera::MoveBy(glm::vec2 shift) {
    cur_pos -= shift;
    UpdateView();
}

void Camera::SetPosition(glm::vec2 pos) {
    cur_pos = -pos;
    UpdateView();
}

void Camera::UpdateViewportSize(int w, int h) {
    projection = glm::ortho(0.f, (float) w, (float) h, 0.f, -1.f, 1.f);
    UpdateUbo();
}

void Camera::UpdateView() {
    glm::mat4 mat(1);
    view = glm::translate(mat, glm::vec3(cur_pos, 0));
    
    UpdateUbo();
}

void Camera::UpdateUbo() {
    auto data = dxgl::Std140(CameraUbo{
        .camera_matrix = projection * view
    });

    ubo.Upload(data);
}