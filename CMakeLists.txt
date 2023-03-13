cmake_minimum_required(VERSION 3.24.3 FATAL_ERROR)

project(Lava VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Debug"
    OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(Lava_BUILD_TESTS ON)
endif()
option(Lava_BUILD_TESTS "Build tests" OFF)
option(Lava_BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

find_package(fmt REQUIRED)
find_package(Boost 1.80 REQUIRED
  COMPONENTS container
)
find_package(nlohmann_json 3.11.2 REQUIRED)

set(Lava_INCLUDE_DIRS
  $<BUILD_INTERFACE:
    ${CMAKE_SOURCE_DIR}/include$<SEMICOLON>
    ${CMAKE_BINARY_DIR}
  >
  $<INSTALL_INTERFACE:include>
)

set(Lava_CONTRIB_INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/contrib>
  $<INSTALL_INTERFACE:include/lava/contrib>
)

configure_file(config.h.in config.h @ONLY)
install(FILES config.h DESTINATION "${CMAKE_SOURCE_DIR}/include/lava")

add_subdirectory(contrib)
add_subdirectory(src)
if(Lava_BUILD_TESTS)
  find_package(Catch2 CONFIG REQUIRED)
  add_subdirectory(test)
endif()
if(Lava_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

message(STATUS "Lava: Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Lava: Test targets:    ${Lava_BUILD_TESTS}")
message(STATUS "Lava: Example targets: ${Lava_BUILD_EXAMPLES}")
