target_sources(containsim_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ActionRouter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BuildInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DragHelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RoomInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HotkeyMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ToolEvent.cpp
    InputToolManager.cpp
    InputTool.cpp
    RootTool.hpp
    RootTool.cpp
    DragTracker.cpp
)
