cmake_minimum_required(VERSION 3.20)

project(dxgl
	VERSION 1.0
	DESCRIPTION "DXGL"
	LANGUAGES CXX C
)

message(STATUS "Compiling with C++ flags: " ${CMAKE_CXX_FLAGS})

find_package(Boost 1.62.0)

if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/install/$<0:>)

enable_testing()

add_subdirectory(${CMAKE_SOURCE_DIR}/dxgl)
add_subdirectory(${CMAKE_SOURCE_DIR}/containsim)
