cmake_minimum_required (VERSION 2.6)
project (Project_JJ_Part1)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -Wall")

include_directories(include)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE histogram.cpp partitioner.cpp test.cpp list.cpp)