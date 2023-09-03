#include <exception>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <format>
#include <concepts>
#include <numbers>

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
#include "Vao.hpp"

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
static bool flashlight_on = true;

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
            case GLFW_KEY_Q:
                flashlight_on ^= true;
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

Vao MakeTris(std::vector<float> vertices) {
    Vao Vao{};
    Vao.Use();

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

    return Vao;
}

void Clear() {
    glClearColor(0.1, 0.2, 0.4, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render(const Program& program, const Vao& Vao) {
    program.Use();
    Vao.Use();

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

    Camera& camera = Camera::Get();
    camera.UpdateWindowSize(800, 600);

    auto cube_program = LoadProgram("shaders/perspective.vert", "shaders/phong_tex_multi.frag");
    
    auto light_program_red = LoadProgram("shaders/perspective.vert", "shaders/light.frag");
    auto light_program_green = LoadProgram("shaders/perspective.vert", "shaders/light.frag");
    auto light_program_blue = LoadProgram("shaders/perspective.vert", "shaders/light.frag");
    auto light_program_yellow = LoadProgram("shaders/perspective.vert", "shaders/light.frag");

    auto cube_vao = MakeTris(cube_vertices);

    std::vector<Cube> cubes;

    glm::vec3 cube_positions[] = {
        glm::vec3( 0.0f,  0.0f, -3.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    for (const auto& pos : cube_positions) {
        cubes.push_back(Cube{
            .vao = cube_vao,
            .program = cube_program,
            .position = pos,
            .rotation = pos
        });
    }

    Cube red_light_cube{
        .vao = cube_vao,
        .program = light_program_red,
        .position = { 3, 1.5, -5 }
    };

    Cube green_light_cube{
        .vao = cube_vao,
        .program = light_program_green,
        .position = { 3, 1.5, -5 }
    };

    Cube blue_light_cube{
        .vao = cube_vao,
        .program = light_program_blue,
        .position = { 3, 1.5, -5 }
    };

    Cube yellow_light_cube{
        .vao = cube_vao,
        .program = light_program_yellow,
        .position = { 3, 1.5, -5 }
    };
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



    auto crate = LoadTextureFromFile("res/img/crate.png");
    auto crate_specular = LoadTextureFromFile("res/img/crate_specular.png");
    // auto matrix = LoadTextureFromFile("res/img/matrix.jpg");
    auto blank = LoadTextureFromFile("res/img/blank.png");

    TexMaterial mat {
        .diffuse_map = crate,
        .specular_map = crate_specular,
        .emission_map = blank,
        .shininess = 0.5
    };

    mat.emission_map->Use(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    DirectionalLight sun{
        .color{
            .ambient{0.3},
            .diffuse{1},
            .specular{1}
        },
        .direction{0, -1, 0}
    };

    LightColor flashlight_color{
        .ambient{0.1},
        .diffuse{1},
        .specular{1, 1, 1}
    }; 

    Spotlight spotlight{
        .color{},
        .inner_cutoff = 12.5f,
        .outer_cutoff = 20.f
    };

    glm::vec3 red{1, 0, 0};
    glm::vec3 green{0, 1, 0};
    glm::vec3 blue{0, 0, 1};
    glm::vec3 yellow{1, 1, 0};

    auto MakePointLight = [](const glm::vec3& color) {
        return PointLight{
            .color = {
                .ambient = color * 0.1f,
                .diffuse = color,
                .specular = color
            },
            .attenuation = Attenuations::D32
        };
    };

    auto red_light = MakePointLight(red);
    auto green_light = MakePointLight(green);
    auto blue_light = MakePointLight(blue);
    auto yellow_light = MakePointLight(yellow);

    Uniform::Set(cube_program, "material", mat);
    Uniform::Set(light_program_red, "light_color", red);
    Uniform::Set(light_program_green, "light_color", green);
    Uniform::Set(light_program_yellow, "light_color", yellow);
    Uniform::Set(light_program_blue, "light_color", blue);

    Uniform::Set(cube_program, "dir_light", sun);

    auto original_light_pos = red_light.position;
    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        ProcessInput(window);
        camera.UpdatePosition(window, delta_time);

        Clear();

        auto RotateAround = [](glm::vec3 pos, float radians) {
            glm::mat4 m = glm::mat4(1);
            m = glm::translate(m, pos);
            m = glm::rotate(m, radians, glm::vec3(0, 1, 0));
            m = glm::translate(m, pos);

            return m;
        };

        float rot = glfwGetTime() * 1.5f;

        red_light_cube.position = red_light.position = RotateAround(
            cubes[0].position, rot
        ) * glm::vec4(original_light_pos, 1);

        green_light_cube.position = green_light.position = RotateAround(
            cubes[0].position, rot + std::numbers::pi / 2
        ) * glm::vec4(original_light_pos, 1);

        blue_light_cube.position = blue_light.position = RotateAround(
            cubes[0].position, rot + std::numbers::pi
        ) * glm::vec4(original_light_pos, 1);

        yellow_light_cube.position = yellow_light.position = RotateAround(
            cubes[0].position, rot + 3 * std::numbers::pi / 2
        ) * glm::vec4(original_light_pos, 1);

        spotlight.position = camera.GetPosition();
        spotlight.direction = camera.GetDirection();

        Uniform::Set(cube_program, "point_lights[0]", red_light);
        Uniform::Set(cube_program, "point_lights[1]", green_light);
        Uniform::Set(cube_program, "point_lights[2]", blue_light);
        Uniform::Set(cube_program, "point_lights[3]", yellow_light);

        spotlight.color = flashlight_on ? flashlight_color : LightColor{};
        Uniform::Set(cube_program, "spotlight", spotlight);

        Uniform::Set(cube_program, "view_pos", camera.GetPosition());

        green_light_cube.Render(camera);
        red_light_cube.Render(camera);
        yellow_light_cube.Render(camera);
        blue_light_cube.Render(camera);

        sun.color.ambient = glm::vec3(mix_value) * 0.1f;
        sun.color.diffuse = glm::vec3(mix_value);
        sun.color.specular = glm::vec3(mix_value);

        Uniform::Set(cube_program, "dir_light", sun);

        for (const auto& cube : cubes) {
            cube.Render(camera);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}