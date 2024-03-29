#include <cstring>

#include "map_info.h"
#include "query_exec.h"
#include "job_scheduler.h"
#include "query_results.h"
#include "config.h"

memory_map rel_mmap[14];

QueryResults qres[70];

JobScheduler js(THREAD_COUNT);
JobScheduler qe_jobs(THREAD_COUNT_QUERIES);

int main(int argc, char* argv[]) {
#ifdef Q_OPT
  fprintf(stderr, "Query Optimizer is ON.\n");
#else
  fprintf(stderr, "Query Optimizer is OFF.\n");
#endif
  fprintf(stderr, "Scheduler is using %d threads.\n", THREAD_COUNT);
  int64_t relations_count = 0;
  char line[4096];
  while (std::fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\r\n")] = '\0';
    if (strcmp(line, "Done") == 0) break;

    rel_mmap[relations_count++] = parse_relation(line);
  }

  int queries = 0;
  int curr_query = 0;
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    if (strcmp(line, "F") == 0) {  // End of a batch

      qe_jobs.wait_all();

      for (; curr_query < queries; curr_query++) {
        for (int j = 0; j < qres[curr_query].projections; j++) {
          (qres[curr_query].sums[j]) ? printf("%ld", qres[curr_query].sums[j])
                                     : printf("NULL");
          if (j < qres[curr_query].projections - 1) printf(" ");
        }
        printf("\n");
      }

      fflush(stdout);

      continue;
    }
    qe_jobs.add_job(new QueryJob(strdup(line), queries++));
  }

  return 0;
}