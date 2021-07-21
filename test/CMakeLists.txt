add_executable(test
  data.cpp
  lexer.cpp
  rope.cpp
  source.cpp
  terminal.cpp
  test.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2
  ashdata
  ashparser
  ashsource
  ashterminal
  rope
)

include(CTest)
include(Catch)
catch_discover_tests(test)
