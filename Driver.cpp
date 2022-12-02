#include <cstring>

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
  }

  QueryExec qe;
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\r\n")] = '\0';
    if (strcmp(line, "F") == 0) continue;
    qe.execute(line);
  }

  return 0;
}