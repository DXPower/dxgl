add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

add_executable(containsim)

target_sources(containsim PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DeltaTimer.cpp
)
target_link_libraries(containsim PUBLIC containsim_core)
target_compile_options(containsim PRIVATE -Wno-unused-parameter)

# target_link_options(containsim PUBLIC /ENTRY:mainCRTStartup /subsystem:windows)

add_custom_target(debug-generators
  COMMAND ${CMAKE_COMMAND} -E echo 
  "target dir = $<TARGET_FILE_DIR:containsim>; bin dir = ${CMAKE_CURRENT_BINARY_DIR} ")