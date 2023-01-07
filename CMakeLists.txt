cmake_minimum_required (VERSION 2.6)
project (Project_JJ)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -Wall")

enable_testing()

include_directories(include)

#unit test
#add_executable(utest hashTable.cpp histogram.cpp list.cpp tests/partitioner_old.cpp tests/join_old.cpp tests/test.cpp )

add_executable(harness harness.cpp)

add_executable(Driver hashTable.cpp histogram.cpp list.cpp partitioner.cpp join.cpp parse_relation.cpp query_exec.cpp Driver.cpp)

target_link_libraries(Driver pthread)


add_test(unit_tests utest)
