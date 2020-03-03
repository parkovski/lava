set(SOURCES
  keywords.cpp
  lexer.cpp
  parser.cpp
  token.cpp
)

add_library(ashparser STATIC ${SOURCES})
target_include_directories(ashparser PUBLIC ${ASH_INCLUDE_DIR})
target_link_libraries(ashparser INTERFACE ashast ashsource)
