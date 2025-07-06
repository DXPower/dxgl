#include <modules/rendering/Camera.hpp>

#include <common/Rendering.hpp>

#include <boost/describe/class.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace rendering;

namespace {
    struct CameraUbo {
        glm::mat4 camera_matrix;
        glm::mat4 projection_matrix;
    };

    BOOST_DESCRIBE_STRUCT(CameraUbo, (), (camera_matrix, projection_matrix));
};

Camera::Camera(dxgl::UboBindingManager& ubos) {
    UpdateView();

    ubos.BindUboLocation(
        static_cast<std::size_t>(UboLocs::Camera), 
        ubo
    );
}

void Camera::MoveBy(glm::vec2 shift) {
    SetPosition(cur_pos - shift);
}

void Camera::LookAt(glm::vec2 pos) {
    SetPosition(pos - ((glm::vec2) viewport_size / 2.f)); // NOLINT
}

void Camera::SetPosition(glm::vec2 pos) {
    cur_pos = pos;
    UpdateView();
}


void Camera::UpdateViewportSize(glm::ivec2 size) {
    viewport_size = size;
    projection = glm::ortho(0.f, (float) size.x, (float) size.y, 0.f, -1.f, 1.f);
    UpdateUbo();
}

void Camera::UpdateView() {
    glm::mat4 mat(1);
    view = glm::translate(mat, glm::vec3(cur_pos, 0));
    
    UpdateUbo();
}

void Camera::UpdateUbo() {
    auto data = dxgl::Std140(CameraUbo{
        .camera_matrix = projection * view,
        .projection_matrix = projection
    });

    ubo.Upload(data);
}