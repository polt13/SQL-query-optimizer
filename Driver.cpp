#include <cstring>
#include <iostream>

#include "map_info.h"
#include "queryParser.h"

int main(int argc, char* argv[]) {
  int64_t relations_count = 0;
  memory_map rel_mmap[14];
  // memory_map* rel_mmap = new memory_map[14];
  //  Read join relations
  char line[4096];
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "Done") == 0) break;
    // std::fprintf(stderr, "%ld\n", relations_count);
    // std::fprintf(stderr, "%s\n", line);
    rel_mmap[relations_count++] = parse_relation(line);
  }
  // Preparation phase (not timed)
  // Build histograms, indexes,...
  //
  QueryExec qe;
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "F") == 0) continue;  // End of a batch
    // std::fprintf(stderr, "%s\n", line);
    qe.execute(line);
  }

  // delete[] rel_mmap;

  return 0;
}