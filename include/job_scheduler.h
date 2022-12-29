#ifndef JOBSCHD_H
#define JOBSCHD_H

#include "simple_queue.h"
#include <pthread.h>
#include "job.h"
#include "config.h"

class JobScheduler {
  pthread_t total_threads[THREAD_COUNT];
  static pthread_mutex_t qmtx;
  static pthread_cond_t cvar;
  static simple_queue<Job*> job_pool;

  static void* do_job(void* v) {
    while (1) {
      pthread_mutex_lock(&qmtx);

      while (job_pool.getLen() == 0) {
        pthread_cond_wait(&cvar, &qmtx);
      }

      // prolong lifetime
      Job* j = job_pool.pop();

      j->run();

      pthread_mutex_unlock(&qmtx);
    }
  }

 public:
  JobScheduler() {
    for (int64_t i = 0; i < THREAD_COUNT; i++)
      pthread_create(total_threads + i, NULL, do_job, NULL);
  }

  void add_job(Job* job) { job_pool.enqueue(job); }

  // wip
  void exec_all() { pthread_cond_broadcast(&cvar); }
};

#endif