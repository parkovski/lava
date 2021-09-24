add_executable(ash ash.cpp eval.cpp)
target_link_libraries(ash ashdata ashterminal ashsource ashparser)

add_executable(ty ty.cpp)
target_link_libraries(ty ashterminal ashty)
