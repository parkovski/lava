add_library(rope INTERFACE IMPORTED GLOBAL)
target_sources(rope INTERFACE rope/rope.c)
target_include_directories(rope INTERFACE ${Lava_CONTRIB_INCLUDE_DIRS})

add_subdirectory(hat-trie)
