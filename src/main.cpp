#include <exception>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <format>
#include <concepts>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <xutility>

#include "Camera.hpp"
#include "Cube.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Uniform.hpp"
#include "VAO.hpp"

static std::vector<float> cube_vertices = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

void OnWindowResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
    Camera::Get().UpdateWindowSize(width, height);
}

static float mix_value = 1.f;

float delta_time = 0.0f;

void ProcessInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void OnInput(GLFWwindow* window [[maybe_unused]], int key, int scancode [[maybe_unused]], int action, int mods [[maybe_unused]]) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_UP:
                mix_value += 0.1;
                break;
            case GLFW_KEY_DOWN:
                mix_value -= 0.1;
                break;
        }
    }
}

Program LoadProgram(std::string_view vs_path, std::string_view fs_path) {
    try {
        Shader vs(ShaderType::Vertex, vs_path);
        Shader fs(ShaderType::Fragment, fs_path);

        return Program({ vs, fs });
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        throw;
    }
}

VAO MakeTris(std::vector<float> vertices) {
    // unsigned int indices[] = {
    //     0, 1, 2,
    //     0, 2, 3
    // };

    VAO vao{};
    vao.Use();

    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // unsigned int ebo;
    // glGenBuffers(1, &ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    return vao;
}

void Clear() {
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render(const Program& program, const VAO& vao) {
    program.Use();
    vao.Use();

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// void SetClipMatrices(const Camera& camera, const Program& program, glm::vec3 model_pos) {
//     using namespace glm;

//     auto model = mat4(1);
//     model = translate(model, model_pos);
//     model = rotate(model, (float) glfwGetTime(), vec3(0.5f, 1.0f, 0.0f));

//     auto view = camera.GetViewMatrix();
//     auto projection = camera.GetProjectionMatrix();

//     program.Use();
//     auto handle = program.GetHandle();

//     int model_loc = glGetUniformLocation(handle, "model");
//     glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

//     int view_loc = glGetUniformLocation(handle, "view");
//     glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

//     int projection_loc = glGetUniformLocation(handle, "projection");
//     glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
// }

void SetLightColor(glm::vec3 color, std::same_as<Cube> auto&... cubes) {
    (Uniform::Set(cubes.program, "light_color", color), ...);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "dxgl", nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, OnWindowResize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }  

    glViewport(0, 0, 800, 600);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, OnInput);

    Cube cube{
        .vao = MakeTris(cube_vertices),
        .program = LoadProgram("shaders/perspective.vert", "shaders/phong.frag"),
        .position = { 1, -1.f, -7 }
    };

    Cube light{
        .vao = MakeTris(cube_vertices),
        .program = LoadProgram("shaders/perspective.vert", "shaders/light.frag"),
        .position = { -1, 0, 1 }
    };

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // auto crate_texture = LoadTextureFromFile("res/img/container.jpg");
    // auto epic_texture = LoadTextureFromFile("res/img/epic.png");

    // program.Use();
    // glUniform1i(glGetUniformLocation(program.GetHandle(), "tex1"), 0);
    // glUniform1i(glGetUniformLocation(program.GetHandle(), "tex2"), 1);


    // glm::vec3 cubes[] = {
    //     glm::vec3( 0.0f,  0.0f,  -5.0f), 
    //     glm::vec3( 2.0f,  5.0f, -15.0f), 
    //     glm::vec3(-1.5f, -2.2f, -2.5f),  
    //     glm::vec3(-3.8f, -2.0f, -12.3f),  
    //     glm::vec3( 2.4f, -0.4f, -3.5f),  
    //     glm::vec3(-1.7f,  3.0f, -7.5f),  
    //     glm::vec3( 1.3f, -2.0f, -2.5f),  
    //     glm::vec3( 1.5f,  2.0f, -2.5f), 
    //     glm::vec3( 1.5f,  0.2f, -1.5f), 
    //     glm::vec3(-1.3f,  1.0f, -1.5f)  
    // };

    Camera& camera = Camera::Get();
    camera.UpdateWindowSize(800, 600);

    float last_time = 0.0f;

    Uniform::Set(cube.program, "object_color", glm::vec3(1.0f, 0.5f, 0.31f));
    Uniform::Set(cube.program, "light_pos", light.position);
    // Uniform::Set(cube.program, "light_color", glm::vec3(1.0f));
    auto original_light_pos = light.position;

    SetLightColor(glm::vec3(mix_value), cube, light);

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        ProcessInput(window);
        camera.UpdatePosition(window, delta_time);

        Clear();

        // crate_texture.Use(0);
        // epic_texture.Use(1);

        // program.Use();
        // glUniform1f(glGetUniformLocation(program.GetHandle(), "mix_value"), mix_value);
        
        // for (const auto& pos : cubes) {
        //     SetClipMatrices(camera, program, pos);
        //     Render(program, cube);
        // }
        // Uniform::Set(cube.program, "light_color", glm::vec3(mix_value));
        SetLightColor(glm::vec3(mix_value), cube, light);

        glm::mat4 m = glm::mat4(1);
        m = glm::translate(m, cube.position);
        m = glm::rotate(m, (float) glfwGetTime(), glm::vec3(0, 1, 0));
        m = glm::translate(m, -cube.position);

        light.position = m * glm::vec4(original_light_pos, 1);

        Uniform::Set(cube.program, "light_pos", light.position);
        Uniform::Set(cube.program, "view_pos", camera.GetPosition());

        cube.Render(camera);
        light.Render(camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}