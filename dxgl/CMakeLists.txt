add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/glad)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/glm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/stb)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/tiny_obj_loader)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dxtl)

add_library(dxgl)
target_include_directories(dxgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(dxgl SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})


target_link_libraries(dxgl 
    PUBLIC 
        dxtl
        glm
    PRIVATE
        glfw
        glad
        stb
        tiny_obj_loader
)

target_sources(dxgl PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Cubemap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Ebo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Framebuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderbuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Screenbuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Texture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/TextureStore.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Ubo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Uniform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Vao.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Vbo.cpp
)
