set(SOURCES rope.cpp)

add_library(ashdata STATIC ${SOURCES})
target_include_directories(ashdata PUBLIC ${ASH_INCLUDE_DIR})

# Statically link to librope as an implementation detail.
target_link_libraries(ashdata PRIVATE rope)
