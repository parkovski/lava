cmake_minimum_required(VERSION 3.14.4 FATAL_ERROR)

project(ash VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
endif()

# find_package(range-v3 CONFIG REQUIRED)

set(ASH_INCLUDE_DIR
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

set(ASH_CONTRIB_INCLUDE_DIR
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/contrib>
  $<INSTALL_INTERFACE:contrib/ash>
)

add_subdirectory(contrib)
add_subdirectory(lib)
add_subdirectory(bin)
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if(BUILD_TESTS)
  find_package(Catch2 CONFIG REQUIRED)
  add_subdirectory(test)
endif()
