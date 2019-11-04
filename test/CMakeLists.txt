add_executable(doc doc.cpp)
target_link_libraries(doc PRIVATE ashdocument ashterminal)

add_executable(test
  test.cpp
  terminal.cpp
  rope.cpp
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
