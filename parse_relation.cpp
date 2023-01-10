#include <fcntl.h>
#include <cstdio>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include "map_info.h"
#include "config.h"

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

  mapper.addr = addr;

  mapper.rows = *(uint64_t*)(addr);
  addr += sizeof(mapper.rows);
  mapper.cols = *(size_t*)(addr);

#ifdef Q_OPT
  // For Query Optimizer - Statistics
  mapper.stats = new statistics[mapper.cols];
#endif

  // create an array to replace the vector in the original file
  // each element is a pointer to a column
  // use the row info to traverse
  mapper.colptr = new uint64_t*[mapper.cols];

  addr += sizeof(size_t);
  for (unsigned i = 0; i < mapper.cols; ++i) {
    mapper.colptr[i] = (uint64_t*)(addr);
    addr += mapper.rows * sizeof(uint64_t);

#ifdef Q_OPT
    // For Query Optimizer - Find and Store Statistics
    uint64_t min = mapper.colptr[i][0];
    uint64_t max = mapper.colptr[i][0];
    for (unsigned j = 0; j < mapper.rows; j++) {
      if (mapper.colptr[i][j] < min) min = mapper.colptr[i][j];
      if (mapper.colptr[i][j] > max) max = mapper.colptr[i][j];
    }
    mapper.stats[i].l = min;
    mapper.stats[i].u = max;
    mapper.stats[i].f = mapper.rows;

    int64_t arr_size = max - min + 1;
    if (arr_size > UPPER_LIMIT) arr_size = UPPER_LIMIT;
    bool* d_array = new bool[arr_size]{};  // initialize all with "false"

    for (unsigned j = 0; j < mapper.rows; j++) {
      uint64_t val = mapper.colptr[i][j];
      d_array[(val - min) % UPPER_LIMIT] = true;
    }
    for (unsigned j = 0; j < arr_size; j++)
      if (d_array[j]) mapper.stats[i].d++;

    delete[] d_array;
#endif
  }

  return mapper;
}