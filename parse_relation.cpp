#include <fcntl.h>
#include <cstdio>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include "map_info.h"

memory_map parse_relation(const char* fileName) {
  int fd = open(fileName, O_RDONLY);
  if (fd == -1) {
    std::perror("Can not open file\n");
    throw;
  }

  // Obtain file size
  struct stat sb;
  if (fstat(fd, &sb) == -1) std::perror("fstat\n");

  auto length = sb.st_size;

  char* addr = (char*)(mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0u));
  if (addr == MAP_FAILED) {
    std::perror("Can not map file\n");
    throw;
  }

  if (length < 16) {
    std::perror("Relation file doesn't contain a valid header\n");
    throw;
  }

  // map using something an equivalent approach to the vector (since no STL)
  memory_map mapper;

  mapper.rows = *(uint64_t*)(addr);
  addr += sizeof(mapper.rows);
  mapper.cols = *(size_t*)(addr);

  // create an array to replace the vector in the original file
  // each element is a pointer to a column
  // use the row info to traverse
  mapper.colptr = new uint64_t*[mapper.cols];

  addr += sizeof(size_t);
  for (unsigned i = 0; i < mapper.cols; ++i) {
    mapper.colptr[i] = (uint64_t*)(addr);
    addr += mapper.rows * sizeof(uint64_t);
  }

  return mapper;
}