set(SOURCES test.cpp rbtree.cpp)

add_executable(test ${SOURCES})
target_link_libraries(test Catch2::Catch2 ashcollections)

# add_executable(doc doc.cpp)
# target_link_libraries(doc ashcollections ashterminal rope)

# add_executable(symtab symtab.cpp)
# target_link_libraries(symtab ashdocument ashparser ashsymbol ashterminal)

# add_executable(intervaltree intervaltree.cpp)
# target_link_libraries(intervaltree ashcollections)

# add_executable(rbtree rbtree.cpp)
# target_link_libraries(rbtree ashcollections)

include(CTest)
include(Catch)
catch_discover_tests(test)
