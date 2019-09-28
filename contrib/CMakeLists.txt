add_library(rope INTERFACE IMPORTED GLOBAL)
target_sources(rope INTERFACE rope/rope.c)
target_include_directories(rope INTERFACE ${ASH_CONTRIB_INCLUDE_DIR})

add_subdirectory(hat-trie)
