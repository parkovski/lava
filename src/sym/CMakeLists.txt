set(SOURCES
  symtab.cpp
)

add_library(ashsym STATIC ${SOURCES})
target_include_directories(ashsym PUBLIC ${ASH_INCLUDE_DIR})
