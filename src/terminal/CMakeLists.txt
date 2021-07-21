set(SOURCES ansi.cpp)

if(WIN32)
  list(APPEND SOURCES terminal-windows.cpp)
elseif(UNIX)
  list(APPEND SOURCES terminal-unix.cpp)
else()
  message(FATAL_ERROR "Unrecognized platform")
endif()

add_library(ashterminal STATIC ${SOURCES})
target_link_libraries(ashterminal fmt::fmt-header-only)
target_include_directories(ashterminal PUBLIC ${ASH_INCLUDE_DIR})
