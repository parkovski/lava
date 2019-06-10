add_executable(doc doc.cpp)
target_link_libraries(doc ashterminal rope intervaltree)

add_executable(symtab symtab.cpp)
target_link_libraries(symtab ashdocument ashparser ashsymbol ashterminal)
