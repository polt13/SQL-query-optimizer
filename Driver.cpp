#include <cstring>
#include <iostream>

#include "map_info.h"
#include "query_exec.h"

memory_map rel_mmap[14];

int main(int argc, char* argv[]) {
  int64_t relations_count = 0;

  char line[4096];
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "Done") == 0) break;

    rel_mmap[relations_count++] = parse_relation(line);
    for (int i = 0; i < 14; i++) {
      for (unsigned j = 0; j < rel_mmap[i].cols; j++) {
        fprintf(stderr, "l%d = %ld\n", j, rel_mmap[i].stats[j].l);
        fprintf(stderr, "u%d = %ld\n", j, rel_mmap[i].stats[j].u);
        fprintf(stderr, "f%d = %ld\n", j, rel_mmap[i].stats[j].f);
        fprintf(stderr, "d%d = %ld\n", j, rel_mmap[i].stats[j].d);
      }
      fprintf(stderr, "\n");
    }
  }

  QueryExec qe;
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "F") == 0) {  // End of a batch
      fflush(stdout);
      continue;
    }
    qe.execute(line);
  }

  return 0;
}