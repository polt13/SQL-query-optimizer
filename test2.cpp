#include <cstdio>
#include "dataForm.h"
#include "map_info.h"

int main() {
  std::printf("testing\n");
  memory_map m = parse_relation("r0");
  for (uint64_t i = 0; i < m.cols; i++) {
    for (uint64_t j = 0; j < m.rows; j++) {
      printf("%ld_\n", m.colptr[i][j]);
    }
  }
}