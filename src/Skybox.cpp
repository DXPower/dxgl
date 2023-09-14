#include "Camera.hpp"
#include "Skybox.hpp"
#include "Uniform.hpp"

#include <glad/glad.h>

void Skybox::Render(const Camera& camera) const {
    vao->Use();
    program->Use();
    cubemap->Use();

    Uniform::Set(*program, "view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
    Uniform::Set(*program, "projection", camera.GetProjectionMatrix());

    glDrawArrays(GL_TRIANGLES, 0, 36);
}