add_subdirectory(commands)

target_sources(containsim_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logging.cpp
)
