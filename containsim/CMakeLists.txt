find_package(spdlog REQUIRED)
find_package(dxfsm REQUIRED)
find_package(kangaru REQUIRED)
find_package(chain REQUIRED)
find_package(flecs REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(magic_enum REQUIRED)
find_package(RmlUi REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/nano)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dep/RmlUi_Backends)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

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

# get_target_property(ULTRALIGHT_INSPECTOR_PATH Ultralight INSPECTOR_PATH)
# add_custom_target(copy-inspector-ui-cs
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${ULTRALIGHT_INSPECTOR_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/ui/inspector
# )
