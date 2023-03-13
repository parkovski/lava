add_executable(test
  arena.cpp
  cliparser.cpp
  ../src/lava/cliparser.cpp
  intervaltree.cpp
  rope.cpp
  slidingindex.cpp
  terminal.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2WithMain
  lava-data
  lava-term
  rope
)

include(CTest)
include(Catch)
catch_discover_tests(test)
