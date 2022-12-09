add_executable(test
  arena.cpp
  cliparser.cpp
  ../src/lava/cliparser.cpp
  intervaltree.cpp
  rope.cpp
  slidingindex.cpp
  symtab.cpp
  terminal.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2WithMain
  lava-data
  lava-syn
  lava-sym
  lava-term
  rope
)

add_executable(test-print test-print.cpp)
target_link_libraries(test-print lava-syn lava-src fmt::fmt)

# add_executable(test-reconstruct test-reconstruct.cpp)
# target_link_libraries(test-reconstruct lava-syn lava-src fmt::fmt)

include(CTest)
include(Catch)
catch_discover_tests(test)
