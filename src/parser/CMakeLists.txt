set(SOURCES
  lexer.cpp
  token.cpp
)

add_library(ashparser STATIC ${SOURCES})
target_include_directories(ashparser PUBLIC ${ASH_INCLUDE_DIR})
target_link_libraries(ashparser INTERFACE ashsrcloc)
