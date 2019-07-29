add_executable(doc doc.cpp)
target_link_libraries(doc ashcollections ashterminal rope)

add_executable(test test.cpp)
target_link_libraries(test Catch2::Catch2 ashcollections)

include(CTest)
include(Catch)
catch_discover_tests(test)
