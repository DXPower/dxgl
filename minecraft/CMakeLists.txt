add_executable(minecraft)

target_link_libraries(minecraft PRIVATE dxgl glad glfw)
target_sources(minecraft PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Block.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
)

add_custom_target(copy-shaders-mc ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
	DEPENDS minecraft
)

add_custom_target(copy-res-mc ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
	DEPENDS minecraft
)