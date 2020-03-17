add_library(ashast STATIC node.cpp)
target_include_directories(ashast PUBLIC ${ASH_INCLUDE_DIR})
target_link_libraries(ashast PRIVATE Boost::container)
target_link_libraries(ashast INTERFACE ashsource)
if(MSVC)
  # How to make this a dependency?
  target_link_options(ashast INTERFACE "/NATVIS:${CMAKE_CURRENT_SOURCE_DIR}/list.natvis")
endif()
