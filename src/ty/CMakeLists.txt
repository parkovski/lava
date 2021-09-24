set(SOURCES ty.c)

add_library(ashty STATIC ${SOURCES})
target_include_directories(ashty PUBLIC ${ASH_INCLUDE_DIR})
