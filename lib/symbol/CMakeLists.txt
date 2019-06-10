add_library(ashsymbol STATIC symtab.cpp)
target_include_directories(ashsymbol PUBLIC ${ASH_INCLUDE_DIR})