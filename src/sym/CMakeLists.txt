set(SOURCES
  symtab.cpp
)

add_library(lava-sym STATIC ${SOURCES})
target_include_directories(lava-sym PUBLIC ${Lava_INCLUDE_DIRS})
target_link_libraries(lava-sym PUBLIC lava-data)
