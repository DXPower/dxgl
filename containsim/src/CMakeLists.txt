add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

add_executable(containsim)

target_sources(containsim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(containsim PUBLIC containsim_core)

add_custom_command(TARGET containsim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Ultralight> $<TARGET_FILE:UltralightCore> $<TARGET_FILE:WebCore> $<TARGET_FILE:AppCore> 
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_custom_target(debug-generators
  COMMAND ${CMAKE_COMMAND} -E echo 
  "target dir = $<TARGET_FILE_DIR:containsim>; bin dir = ${CMAKE_CURRENT_BINARY_DIR} ")