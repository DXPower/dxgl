add_executable(demo)

target_link_libraries(demo PRIVATE dxgl glad glfw)
target_sources(demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_custom_target(copy-shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
	DEPENDS demo
)

add_custom_target(copy-res ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
	DEPENDS demo
)