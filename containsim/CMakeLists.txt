add_executable(containsim)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/flecs)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/nlohmann_json)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/magic_enum)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/ultralight)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/ultralight_dbg)

target_link_libraries(containsim PRIVATE 
    dxgl
    glad
    glfw
    flecs::flecs_static
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
    Ultralight
    UltralightCore
    WebCore
    AppCore
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/modules)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/services)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/systems)

target_include_directories(containsim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(containsim PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
)

add_custom_target(copy-shaders-cs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
	DEPENDS containsim
)

add_custom_target(copy-res-cs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
	DEPENDS containsim
)

add_custom_target(copy-game-ui-cs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res/ui/game ${CMAKE_CURRENT_BINARY_DIR}/res/ui/game
	DEPENDS containsim
)