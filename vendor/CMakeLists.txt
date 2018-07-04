if (BRICA_TEST AND NOT TARGET gtest)
  add_subdirectory(googletest)
endif()

if (BRICA_BENCH AND NOT TARGET benchmark)
  add_subdirectory(benchmark)
endif()
