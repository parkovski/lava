add_executable(test
  data.cpp
  lexer.cpp
  rope.cpp
  source.cpp
  symtab.cpp
  terminal.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2WithMain
  ashdata
  ashparser
  ashsource
  ashsym
  ashterminal
  rope
)

include(CTest)
include(Catch)
catch_discover_tests(test)
