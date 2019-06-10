add_library(ashparser STATIC lexer.cpp parser.cpp)
target_include_directories(ashparser PUBLIC ${ASH_INCLUDE_DIR})