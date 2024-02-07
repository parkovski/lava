add_executable(test
  data/arena.cpp
  data/intervaltree.cpp
  data/rope.cpp
  data/slidingindex.cpp

  driver/cliparser.cpp
  ../src/driver/cliparser.cpp

  lang/firstpass.cpp
  lang/lexer.cpp
  lang/parser.cpp
  lang/symbol.cpp

  term/terminal.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2WithMain
  lava-data
  lava-lang
  lava-term
  rope
)

add_executable(print-tokens print-tokens.cpp)
target_link_libraries(print-tokens lava-lang fmt::fmt)

add_executable(print-syntax print-syntax.cpp)
target_link_libraries(print-syntax lava-lang fmt::fmt)

add_executable(print-ops print-ops.cpp)
target_link_libraries(print-ops lava-lang fmt::fmt)

add_executable(print-ir print-ir.cpp)
target_link_libraries(print-ir lava-lang fmt::fmt)

include(CTest)
include(Catch)
catch_discover_tests(test)
