set(SOURCES
  lexer.cpp
  #parser.cpp
)

add_library(lava-syn STATIC ${SOURCES})
target_include_directories(lava-syn PUBLIC ${Lava_INCLUDE_DIRS})
target_link_libraries(lava-syn PUBLIC Boost::container)
