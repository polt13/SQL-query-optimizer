#ifndef MAP_INFO
#define MAP_INFO

#include <cstdint>

struct statistics {
  uint64_t l;      // minimum
  uint64_t u;      // maximum
  uint64_t f;      // count
  uint64_t d = 0;  // distinct count
};

struct memory_map {
  // addr mapped for unmapping later
  void* addr;
  size_t cols;
  uint64_t rows;
  uint64_t** colptr;
  statistics* stats;  // For query optimizer
};

extern memory_map rel_mmap[14];

memory_map parse_relation(const char*);

#endif