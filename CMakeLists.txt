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

# Build tests (if enabled)
if (BRICA_TEST)
  enable_testing()
  add_executable(unit_tests tests/main.cpp tests/assocvec.cpp tests/component.cpp tests/scheduler.cpp)
  add_test(NAME brica-test COMMAND unit_tests)
endif()

INSTALL(FILES
  include/brica/assocvec.hpp
  include/brica/brica.hpp
  include/brica/buffer.hpp
  include/brica/component.hpp
  include/brica/functor.hpp
  include/brica/mpi.hpp
  include/brica/scheduler.hpp
  DESTINATION include/brica)
