#ifndef MAP_INFO
#define MAP_INFO

#include <cstdint>

struct memory_map {
  // addr mapped for unmapping later
  void* addr;
  size_t cols;
  uint64_t rows;
  uint64_t** colptr;
};

memory_map parse_relation(const char*);

#endif