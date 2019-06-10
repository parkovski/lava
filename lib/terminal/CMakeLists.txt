set(SOURCES lineeditor.cpp)

if(WIN32)
  list(APPEND SOURCES terminal-windows.cpp)
elseif(UNIX)
  list(APPEND SOURCES terminal-unix.cpp)
else()
  message(FATAL_ERROR "Unrecognized platform")
endif()

add_library(ashterminal STATIC ${SOURCES})
target_include_directories(ashterminal PUBLIC ${ASH_INCLUDE_DIR})