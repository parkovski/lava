add_executable(test
  arena.cpp
  cliparser.cpp
  ../src/driver/cliparser.cpp
  intervaltree.cpp
  lexer.cpp
  parser.cpp
  resolver.cpp
  rope.cpp
  slidingindex.cpp
  # symtab.cpp
  terminal.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2WithMain
  lava-data
  lava-syn
  lava-term
  rope
)

include(CTest)
include(Catch)
catch_discover_tests(test)
