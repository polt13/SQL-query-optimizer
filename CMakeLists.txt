cmake_minimum_required (VERSION 2.6)
project (Project_JJ_Part1)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -Wall")

enable_testing()

include_directories(include)

add_executable(${PROJECT_NAME} hashTable.cpp histogram.cpp list.cpp partitioner.cpp join.cpp parse_relation.cpp)

add_test(unit_tests ${PROJECT_NAME})
