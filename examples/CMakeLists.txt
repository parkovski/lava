add_executable(editor editor.cpp)
target_link_libraries(editor ashterminal ashdocument)

add_executable(lineedit lineedit.cpp)
target_link_libraries(lineedit ashterminal ashdocument)

# add_executable(symtab symtab.cpp)
# target_link_libraries(symtab ashterminal fmt::fmt-header-only tsl::hat_trie)
