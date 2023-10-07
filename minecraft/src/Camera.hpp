#include "GlobalState.hpp"

#include <dxgl/Ubo.hpp>
#include <glm/glm.hpp>

class Camera {
    glm::vec2 cur_pos{};

    dxgl::Ubo ubo{};

    glm::mat4 projection{};
    glm::mat4 view{};

public:
    Camera(GlobalState& global_state);

    void MoveBy(glm::vec2 shift);
    void SetPosition(glm::vec2 pos);

    glm::vec2 GetPosition() const { return cur_pos; }

    void UpdateViewportSize(int w, int h);

private:
    void UpdateView();
    void UpdateUbo();
};