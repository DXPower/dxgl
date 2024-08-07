add_library(containsim_core)

target_link_libraries(containsim_core PUBLIC 
    chain
    dxgl
    dxfsm
    glad
    glfw
    flecs::flecs_static
    kangaru
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
    spdlog::spdlog
    cofsm
    nano
    Ultralight
    UltralightCore
    WebCore
    AppCore
)

target_compile_definitions(containsim_core PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/services)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/systems)

target_include_directories(containsim_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
