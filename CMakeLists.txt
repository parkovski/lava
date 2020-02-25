cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(ash VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

# find_package(range-v3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost 1.72 REQUIRED
  COMPONENTS container
)

set(ASH_INCLUDE_DIR
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

set(ASH_CONTRIB_INCLUDE_DIR
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/contrib>
  $<INSTALL_INTERFACE:contrib/ash>
)

if(MSVC)
  # C4706 - assignment within conditional.
  add_compile_options(/W4 /wd4706 /diagnostics:caret)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -pedantic)
endif()

if(CMAKE_GENERATOR STREQUAL "Ninja")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()

add_subdirectory(contrib)
add_subdirectory(src)
if(BUILD_TESTS)
  find_package(Catch2 CONFIG REQUIRED)
  add_subdirectory(test)
endif()
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
