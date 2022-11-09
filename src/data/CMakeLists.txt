set(SOURCES
  arena.c
  rope.cpp
)

add_library(lava-data STATIC ${SOURCES})
target_include_directories(lava-data PUBLIC ${Lava_INCLUDE_DIRS})

# Statically link to librope as an implementation detail.
target_link_libraries(lava-data PRIVATE rope)
