if(MSVC)
  # C4706 - assignment within conditional.
  add_compile_options(/W4 /wd4706)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -pedantic)
endif()

add_subdirectory(data)
add_subdirectory(driver)
add_subdirectory(lang)
add_subdirectory(term)
