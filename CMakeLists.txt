cmake_minimum_required(VERSION 3.20)

project(dxgl
	VERSION 1.0
	DESCRIPTION "DXGL"
	LANGUAGES CXX C
)

message(STATUS ${CMAKE_CXX_FLAGS})

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/dep/lib/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/dep/inc"
)

add_library(tiny_obj_loader)
target_include_directories(tiny_obj_loader PUBLIC ${CMAKE_SOURCE_DIR}/dep/inc)
target_sources(tiny_obj_loader PRIVATE ${CMAKE_SOURCE_DIR}/dep/src/tiny_obj_loader.cpp)
target_compile_definitions(tiny_obj_loader PUBLIC TINYOBJLOADER_USE_MAPBOX_EARCUT)
set_target_properties(tiny_obj_loader PROPERTIES LINKER_LANGUAGE CXX)

add_library(glad)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/dep/inc)
target_sources(glad PRIVATE ${CMAKE_SOURCE_DIR}/dep/src/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

add_library(stb)
target_include_directories(stb PUBLIC ${CMAKE_SOURCE_DIR}/dep/inc)
target_sources(stb PRIVATE ${CMAKE_SOURCE_DIR}/dep/src/stb_image.c)
set_target_properties(stb PROPERTIES LINKER_LANGUAGE C)


add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_SOURCE_DIR}/dep/inc)
target_compile_definitions(glm INTERFACE GLM_FORCE_XYZW_ONLY)

add_executable(dxgl)
target_include_directories(dxgl PUBLIC ${CMAKE_SOURCE_DIR}/dep/inc ${CMAKE_SOURCE_DIR}/src/inc)
target_link_libraries(dxgl PRIVATE 
    glfw
	glad
	glm
	stb
	tiny_obj_loader
)

target_sources(dxgl PRIVATE
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/Camera.cpp
	${CMAKE_SOURCE_DIR}/src/Cube.cpp
	${CMAKE_SOURCE_DIR}/src/Light.cpp
	${CMAKE_SOURCE_DIR}/src/Material.cpp
	${CMAKE_SOURCE_DIR}/src/Mesh.cpp
	${CMAKE_SOURCE_DIR}/src/Shader.cpp
	${CMAKE_SOURCE_DIR}/src/Texture.cpp
	${CMAKE_SOURCE_DIR}/src/Uniform.cpp
	${CMAKE_SOURCE_DIR}/src/Vao.cpp
)

add_custom_target(copy-shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
	DEPENDS dxgl
)

add_custom_target(copy-res ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res
	DEPENDS dxgl
)