set(SOURCES
  lexer.cpp
  parser-base.cpp
  parser-ir.cpp
  parser.cpp
  printer.cpp
  #  symgen.cpp
  syntax.cpp
  token.cpp
)

add_library(lava-syn STATIC ${SOURCES})
target_include_directories(lava-syn PUBLIC ${Lava_INCLUDE_DIRS})
target_link_libraries(lava-syn PUBLIC
  lava-data
  lava-src
  # lava-sym
  fmt::fmt
)
