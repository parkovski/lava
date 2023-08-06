set(SOURCES
  lexer.cpp
)

add_library(lava-lang STATIC ${SOURCES})
target_include_directories(lava-lang PUBLIC ${Lava_INCLUDE_DIRS})
target_link_libraries(lava-lang PUBLIC Boost::container)
