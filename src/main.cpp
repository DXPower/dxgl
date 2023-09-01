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
#include "Light.hpp"
#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Uniform.hpp"
#include "VAO.hpp"

static std::vector<float> cube_vertices = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
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
    VAO vao{};
    vao.Use();

    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

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
        .program = LoadProgram("shaders/perspective.vert", "shaders/phong_tex.frag"),
        .position = { 1, -1, -5 }
    };

    Cube light{
        .vao = MakeTris(cube_vertices),
        .program = LoadProgram("shaders/perspective.vert", "shaders/light.frag"),
        .position = { 3, 1.5, -5 }
    };

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // auto crate_texture = LoadTextureFromFile("res/img/container.jpg");
    // auto epic_texture = LoadTextureFromFile("res/img/epic.png");

    // program.Use();
    // glUniform1i(glGetUniformLocation(program.GetHandle(), "tex1"), 0);
    // glUniform1i(glGetUniformLocation(program.GetHandle(), "tex2"), 1);

    Camera& camera = Camera::Get();
    camera.UpdateWindowSize(800, 600);

    float last_time = 0.0f;

    // Material mat = Materials::gold;
    TexMaterial mat {
        .diffuse_map = LoadTextureFromFile("res/img/crate.png"),
        .specular_map = LoadTextureFromFile("res/img/crate_specular.png"),
        .emission_map = LoadTextureFromFile("res/img/matrix.jpg"),
        .shininess = 0.5
    };

    mat.emission_map.Use(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    Light light_props{
        .position = light.position,
        .ambient{0.3},
        .diffuse{1},
        .specular{1, 1, 1}
    };

    Uniform::Set(cube.program, "material", mat);
    Uniform::Set(cube.program, "light", light_props);
    Uniform::Set(light.program, "light_color", glm::vec3(1));

    

    auto original_light_pos = light.position;

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

        glm::mat4 m = glm::mat4(1);
        m = glm::translate(m, cube.position);
        m = glm::rotate(m, (float) glfwGetTime() * 1.5f, glm::vec3(0, 1, 0));
        m = glm::translate(m, -cube.position);

        light.position = light_props.position = m * glm::vec4(original_light_pos, 1);

        // glm::vec3 light_color;
        // light_color.x = sin(glfwGetTime() * 2.0f);
        // light_color.y = sin(glfwGetTime() * 0.7f);
        // light_color.z = sin(glfwGetTime() * 1.3f);
        
        // glm::vec3 diffuse_color = light_color; 
        // glm::vec3 ambient_color = diffuse_color; 

        // light_props.ambient = ambient_color;
        // light_props.diffuse = diffuse_color;
        Uniform::Set(cube.program, "time", (float) glfwGetTime());
        Uniform::Set(cube.program, "light", light_props);
        // Uniform::Set(light.program, "light_color", light_color);
        Uniform::Set(cube.program, "view_pos", camera.GetPosition());

        cube.Render(camera);
        light.Render(camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}