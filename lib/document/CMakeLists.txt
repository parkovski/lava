add_library(ashdocument STATIC document.cpp)
target_include_directories(ashdocument PUBLIC ${ASH_INCLUDE_DIR})
target_link_libraries(ashdocument ashcollections)
