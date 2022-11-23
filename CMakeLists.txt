cmake_minimum_required (VERSION 2.6)
project (Project_JJ)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -Wall")

include_directories(include)

add_executable(Part1 hashTable.cpp histogram.cpp list.cpp partitioner.cpp test.cpp join.cpp)
add_executable(harness harness.cpp)
add_executable(Driver Driver.cpp parse_relation.cpp)