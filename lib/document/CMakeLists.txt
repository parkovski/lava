add_library(ashdocument INTERFACE)
target_include_directories(ashdocument INTERFACE ${ASH_INCLUDE_DIR})
target_link_libraries(ashdocument INTERFACE ashcollections rope)
