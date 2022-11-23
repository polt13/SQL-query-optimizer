#include <cstring>
#include <iostream>

#include "map_info.h"

int main(int argc, char* argv[]) {
  int64_t relations_count = 0;
  memory_map array[14];
  // memory_map* array = new memory_map[14];
  //  Read join relations
  char line[4096];
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "Done") == 0) break;
    // std::fprintf(stderr, "%ld\n", relations_count);
    // std::fprintf(stderr, "%s\n", line);
    array[relations_count++] = parse_relation(line);
  }
  // Preparation phase (not timed)
  // Build histograms, indexes,...
  //
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "F") == 0) continue;  // End of a batch
    std::fprintf(stderr, "%s\n", line);
  }

  // delete[] array;

  return 0;
}