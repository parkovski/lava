add_library(ashsource STATIC locator.cpp lexer.cpp parser.cpp document.cpp)
target_include_directories(ashsource PUBLIC ${ASH_INCLUDE_DIR})
