#include "Cube.hpp"
#include "Uniform.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Cube::Render(const Camera& camera) const {
    using namespace glm;

    program->Use();
    vao->Use();

    auto model = mat4(1);
    model = translate(model, position);
    model = rotate(model, rotation.x, vec3(1, 0, 0));
    model = rotate(model, rotation.y, vec3(0, 1, 0));
    model = rotate(model, rotation.z, vec3(0, 0, 1));
    model = glm::scale(model, this->scale);

    auto view = camera.GetViewMatrix();
    auto projection = camera.GetProjectionMatrix();

    Uniform::Set(*program, "model", model);
    Uniform::Set(*program, "view", view);
    Uniform::Set(*program, "projection", projection);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}