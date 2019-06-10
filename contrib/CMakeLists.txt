add_library(rope INTERFACE IMPORTED GLOBAL)
target_sources(rope INTERFACE rope/rope.c)
target_include_directories(rope INTERFACE ${ASH_CONTRIB_INCLUDE_DIR})

add_library(intervaltree INTERFACE IMPORTED GLOBAL)
target_include_directories(intervaltree INTERFACE ${ASH_CONTRIB_INCLUDE_DIR})