set(SOURCES
  cliparser.cpp
  lava.cpp
  options.cpp
)
add_executable(lava ${SOURCES})
target_link_libraries(lava lava-data fmt::fmt Boost::headers)
