cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(ash VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options(-diagnostics:caret)
endif()

# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# find_package(rope CONFIG REQUIRED)

set(SOURCES main.cpp)
list(TRANSFORM SOURCES PREPEND "src/")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(ash ${SOURCES})
