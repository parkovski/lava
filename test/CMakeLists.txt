add_executable(test
  data.cpp
  document.cpp
  rope.cpp
  terminal.cpp
  test.cpp
)
target_link_libraries(test PRIVATE
  Catch2::Catch2
  ashdata
  ashdocument
  rope
  ashterminal
)

include(CTest)
include(Catch)
catch_discover_tests(test)
