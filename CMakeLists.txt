cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(ash VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# find_package(rope CONFIG REQUIRED)

set(SOURCES lexer.cpp lineeditor.cpp)

if(WIN32)
  list(APPEND SOURCES terminal-windows.cpp)
elseif(UNIX)
  list(APPEND SOURCES terminal-unix.cpp)
else()
  message(FATAL_ERROR "Unrecognized platform")
endif()

list(TRANSFORM SOURCES PREPEND "src/")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(ashlib STATIC ${SOURCES})
add_executable(ash src/main.cpp)
target_link_libraries(ash ashlib)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
