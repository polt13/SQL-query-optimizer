#include <cstring>

#include "map_info.h"
#include "query_exec.h"

memory_map rel_mmap[14];

int main(int argc, char* argv[]) {
  // int64_t relations_count = 0;

  // char line[4096];
  // while (fgets(line, sizeof(line), stdin)) {
  //   line[strcspn(line, "\n")] = '\0';
  //   if (strcmp(line, "Done") == 0) break;

  //   rel_mmap[relations_count++] = parse_relation(line);
  // }

  // QueryExec qe;
  // while (fgets(line, sizeof(line), stdin)) {
  //   line[strcspn(line, "\n")] = '\0';
  //   if (strcmp(line, "F") == 0) continue;  // End of a batch
  //   // std::fprintf(stderr, "%s\n", line);
  //   qe.execute(line);
  // }

  // delete[] rel_mmap;

  QueryExec qe;
  char query[] =
      "3 0 1 |3.2=0.0&3.1=1.0&3.2>3499&3.9<1.0&& 1 > 5.2 && 5 = 1.3|1.2 0.1 "
      "5.3";
  qe.execute(query);

  return 0;
}