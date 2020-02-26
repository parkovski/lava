add_executable(test
  data.cpp
  document.cpp
  lexer.cpp
  rope.cpp
  srcloc.cpp
  terminal.cpp
  test.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2
  ashdata
  ashdocument
  ashparser
  ashsrcloc
  ashterminal
  rope
)

include(CTest)
include(Catch)
catch_discover_tests(test)
