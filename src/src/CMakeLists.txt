set(SOURCES
  source.cpp
)
add_library(lava-src STATIC ${SOURCES})
target_include_directories(lava-src PUBLIC ${Lava_INCLUDE_DIRS})
target_link_libraries(lava-src PUBLIC lava-data)
