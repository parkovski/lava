cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(ash VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE MATCHES "Deb")
  set(BUILD_TESTS ON)
  #set(BUILD_EXAMPLES ON)
endif()
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

find_package(fmt REQUIRED)
find_package(Boost 1.72 REQUIRED COMPONENTS container)
if(WIN32)
  set(uriparser_COMPONENTS char wchar_t)
else()
  set(uriparser_COMPONENTS char)
endif()
find_package(uriparser 0.9.7 REQUIRED COMPONENTS ${uriparser_COMPONENTS})

set(ASH_INCLUDE_DIR
  $<BUILD_INTERFACE:
    ${CMAKE_SOURCE_DIR}/include$<SEMICOLON>
    ${CMAKE_BINARY_DIR}
  >
  $<INSTALL_INTERFACE:include>
)

set(ASH_CONTRIB_INCLUDE_DIR
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/contrib>
  $<INSTALL_INTERFACE:include/ash/contrib>
)

if(MSVC)
  # C4706 - assignment within conditional.
  add_compile_options(/W4 /wd4706 /diagnostics:caret)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -pedantic)
endif()

configure_file(config.h.in config.h @ONLY)
install(FILES config.h DESTINATION "${CMAKE_SOURCE_DIR}/include/ash")

add_subdirectory(contrib)
add_subdirectory(src)
if(BUILD_TESTS)
  find_package(Catch2 CONFIG REQUIRED)
  add_subdirectory(test)
endif()
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
