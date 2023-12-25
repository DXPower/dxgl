#include "common/GlobalState.hpp"

#include <dxgl/Ubo.hpp>
#include <glm/glm.hpp>

class Camera {
    glm::vec2 cur_pos{};

    dxgl::Ubo ubo{};

    glm::ivec2 viewport_size{};
    glm::mat4 projection{};
    glm::mat4 view{};

public:
    Camera(GlobalState& global_state);

    void MoveBy(glm::vec2 shift);
    void LookAt(glm::vec2 pos);
    void SetPosition(glm::vec2 pos);

    glm::vec2 GetPosition() const { return cur_pos; }
    const glm::mat4& GetProjectionMatrix() const { return projection; }
    const glm::mat4& GetViewMatrix() const { return view; }
    
    void UpdateViewportSize(glm::ivec2 size);

private:
    void UpdateView();
    void UpdateUbo();
};