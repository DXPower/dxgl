#include "Cube.hpp"
#include "Uniform.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Cube::Render(const Camera& camera) const {
    using namespace glm;

    program.Use();
    vao.Use();

    auto model = mat4(1);
    // model = rotate(model, rotation);
    // model = rotate(model, rotation.x, vec3(1, 0, 0));
    // model = rotate(model, rotation.y, vec3(0, 1, 0));
    // model = rotate(model, rotation.z, vec3(0, 0, 1));
    model = translate(model, position);
    // rotate

    auto view = camera.GetViewMatrix();
    auto projection = camera.GetProjectionMatrix();

    Uniform::Set(program, "model", model);
    Uniform::Set(program, "view", view);
    Uniform::Set(program, "projection", projection);
    // int model_loc = glGetUniformLocation(handle, "model");
    // glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

    // int view_loc = glGetUniformLocation(handle, "view");
    // glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

    // int projection_loc = glGetUniformLocation(handle, "projection");
    // glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, 36);
}