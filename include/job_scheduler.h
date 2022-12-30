#ifndef JOBSCHD_H
#define JOBSCHD_H

#include "simple_queue.h"
#include <pthread.h>
#include "job.h"
#include "config.h"

extern pthread_barrier_t waitb;

class JobScheduler {
  pthread_t total_threads[THREAD_COUNT];
  static pthread_mutex_t qmtx;
  static pthread_cond_t eq;
  static pthread_cond_t jobs_done;
  static simple_queue<Job*> job_pool;
  static pthread_barrier_t waitb;

  static void* do_job(void* v) {
    while (1) {
      pthread_mutex_lock(&qmtx);

      while (job_pool.getLen() == 0) {
        pthread_cond_wait(&eq, &qmtx);
      }

      // prolong lifetime
      Job* j = job_pool.pop();

      pthread_mutex_unlock(&qmtx);

      j->run();

      delete j;

      pthread_barrier_wait(&waitb);
    }
  }

 public:
  JobScheduler() {
    pthread_barrier_init(&waitb, NULL, THREAD_COUNT + 1);
    for (int64_t i = 0; i < THREAD_COUNT; i++)
      pthread_create(total_threads + i, NULL, do_job, NULL);
  }

  void add_job(Job* job) {
    pthread_mutex_lock(&qmtx);
    job_pool.enqueue(job);
    pthread_cond_broadcast(&eq);
    pthread_mutex_unlock(&qmtx);
  }

  void wait_all() { pthread_barrier_wait(&waitb); }
};

#endif