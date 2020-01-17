set(SOURCES
  document.cpp
  documentcursor.cpp
  stringcursor.cpp
)

add_library(ashdocument STATIC ${SOURCES})
target_include_directories(ashdocument PUBLIC ${ASH_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries(ashdocument PUBLIC ashdata)
