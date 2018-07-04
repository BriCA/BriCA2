cmake_minimum_required(VERSION 2.8)

option(BRICA_TEST "build tests" ON)
option(BRICA_BENCH "build benchmarks" OFF)

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

add_subdirectory(vendor)
add_subdirectory(tests)
