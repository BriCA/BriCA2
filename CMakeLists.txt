cmake_minimum_required(VERSION 2.8)

option(BRICA_TEST "build tests" ON)
option(BRICA_MPI_TEST "build mpi tests" OFF)

set(PROJECT_NAME BriCA)
project(${PROJECT_NAME})
set(BRICA_VERSION_MAJOR 1)
set(BRICA_VERSION_MINOR 0)
set(BRICA_VERSION_PATCH 0)

# Enable C++11
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" STD_CXX11)
if(STD_CXX11)
  add_definitions("-std=c++11")
else()
  add_definitions("-std=c++0x")
endif()

# Project include directory
include_directories(include)

# Add the vendored code
add_subdirectory(vendor)

# Build tests (if enabled)
if (BRICA_TEST)
  enable_testing()
  include_directories(${gtest_SOURCE_DIR}/include)
  add_executable(unit_tests tests/assocvec.cpp tests/component.cpp tests/scheduler.cpp)
  target_link_libraries(unit_tests gtest gtest_main)
  add_test(NAME brica-test COMMAND unit_tests)
endif()

if (BRICA_MPI_TEST)
  enable_testing()
  include_directories(${gtest_SOURCE_DIR}/include)
  add_executable(mpi_tests tests/mpi.cpp)
  target_link_libraries(mpi_tests gtest gtest_main)
endif()
