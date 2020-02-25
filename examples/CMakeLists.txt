# add_executable(editor editor.cpp)
# target_link_libraries(editor ashterminal ashdocument)

add_executable(lineedit lineedit.cpp)
target_link_libraries(lineedit ashterminal ashdocument)

# add_executable(symtab symtab.cpp)
# target_link_libraries(symtab ashterminal ashdocument ashobm)

add_executable(tvm tvm.cpp)
target_link_libraries(tvm
  ashterminal ashdocument ashdata fmt::fmt-header-only Boost::container
)
