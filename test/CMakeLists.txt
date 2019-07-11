add_executable(doc doc.cpp)
target_link_libraries(doc ashcollections ashterminal rope)

add_executable(symtab symtab.cpp)
target_link_libraries(symtab ashdocument ashparser ashsymbol ashterminal)

add_executable(intervaltree intervaltree.cpp)
target_link_libraries(intervaltree ashcollections)
