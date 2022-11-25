cmake_minimum_required (VERSION 2.6)
project (Project_JJ)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -Wall")

enable_testing()

include_directories(include)

# For Part1
add_executable(Part1 hashTable.cpp histogram.cpp list.cpp partitioner.cpp tests/test.cpp join.cpp)

# For Part2
add_executable(harness harness.cpp)
add_executable(Driver hashTable.cpp histogram.cpp list.cpp partitioner.cpp join.cpp parse_relation.cpp queryParser.cpp Driver.cpp)

add_test(unit_tests ${PROJECT_NAME})
