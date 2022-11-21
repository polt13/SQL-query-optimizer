#ifndef MAP_INFO
#define MAP_INFO
struct memory_map {
  size_t cols;
  uint64_t rows;
  uint64_t** colptr;
};

memory_map parse_relation(const char*);

#endif