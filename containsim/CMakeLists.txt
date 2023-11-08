add_executable(containsim)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/flecs)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/nlohmann_json)

target_link_libraries(containsim PRIVATE dxgl glad glfw flecs::flecs_static nlohmann_json::nlohmann_json)

target_include_directories(containsim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(containsim PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/SpriteRenderer.cpp
)

add_custom_target(copy-shaders-cs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
	DEPENDS containsim
)

add_custom_target(copy-res-cs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
	DEPENDS containsim
)