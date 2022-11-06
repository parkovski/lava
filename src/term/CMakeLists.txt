set(SOURCES ansi.cpp)

if(WIN32)
  list(APPEND SOURCES terminal-windows.cpp)
elseif(UNIX)
  list(APPEND SOURCES terminal-unix.cpp)
else()
  message(FATAL_ERROR "Unrecognized platform")
endif()

add_library(lava-term STATIC ${SOURCES})
target_link_libraries(lava-term fmt::fmt)
target_include_directories(lava-term PUBLIC ${Lava_INCLUDE_DIRS})
