add_subdirectory(commands)
add_subdirectory(ui)

target_sources(containsim_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ActionRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BuildInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BuildManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/GameState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TileGrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TileGridRenderer.cpp
)
