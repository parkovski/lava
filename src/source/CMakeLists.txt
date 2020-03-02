add_library(ashsource STATIC locator.cpp)
target_include_directories(ashsource PUBLIC ${ASH_INCLUDE_DIR})
