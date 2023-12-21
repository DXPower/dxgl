add_executable(containsim)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/flecs)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/nlohmann_json)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/magic_enum)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/ultralight)

find_package(spdlog REQUIRED)

target_link_libraries(containsim PRIVATE 
    dxgl
    glad
    glfw
    flecs::flecs_static
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
    spdlog::spdlog
    Ultralight
    UltralightCore
    WebCore
    AppCore
)

target_compile_definitions(containsim PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

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
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
	DEPENDS containsim
)

add_custom_target(copy-res-cs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res
)

add_custom_target(copy-game-ui-cs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res/ui/game ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/ui/game
)

get_target_property(ULTRALIGHT_INSPECTOR_PATH Ultralight INSPECTOR_PATH)
add_custom_target(copy-inspector-ui-cs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ULTRALIGHT_INSPECTOR_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/ui/inspector
)

add_custom_command(TARGET containsim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Ultralight> $<TARGET_FILE:UltralightCore> $<TARGET_FILE:WebCore> $<TARGET_FILE:AppCore> 
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)


add_custom_target(debug-generators
  COMMAND ${CMAKE_COMMAND} -E echo 
  "target dir = $<TARGET_FILE_DIR:containsim>; bin dir = ${CMAKE_CURRENT_BINARY_DIR} ")