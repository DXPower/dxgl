find_package(Catch2 REQUIRED)

add_executable(containsim_tests)

target_sources(containsim_tests PRIVATE 
    BuildMode.cpp
)

target_link_libraries(containsim_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(containsim_tests)
