cmake_minimum_required(VERSION 3.13)

project(BIGINT)

set(CMAKE_CXX_STANDARD 17)

add_executable(vector_testing
               vector_testing.cpp
               vector.h
               mman-win32.h
               mman.cpp
               counted.h
               counted.cpp
               fault_injection.h
               fault_injection.cpp
               gtest/gtest-all.cc
               gtest/gtest.h
               gtest/gtest_main.cc)
