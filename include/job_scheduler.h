#ifndef JOBSCHD_H
#define JOBSCHD_H

#include "simple_queue.h"
#include <pthread.h>
#include "job.h"
#include "config.h"

class JobScheduler {
  pthread_t total_threads[THREAD_COUNT];
  static pthread_mutex_t qmtx;
  static pthread_cond_t eq;
  static pthread_cond_t jobs_done;
  static simple_queue<Job*> job_pool;
  static int busy;

  static void* do_job(void* v) {
    while (1) {
      pthread_mutex_lock(&qmtx);

      while (job_pool.getLen() == 0) {
        pthread_cond_wait(&eq, &qmtx);
      }

      Job* j = job_pool.pop();

      ++busy;

      pthread_mutex_unlock(&qmtx);

      j->run();

      pthread_mutex_lock(&qmtx);

      if (--busy == 0 && job_pool.getLen() == 0)
        pthread_cond_signal(&jobs_done);

      pthread_mutex_unlock(&qmtx);

      delete j;
    }
  }

 public:
  JobScheduler() {
    for (int64_t i = 0; i < THREAD_COUNT; i++)
      pthread_create(total_threads + i, NULL, do_job, NULL);
  }

  void add_job(Job* job) {
    pthread_mutex_lock(&qmtx);
    job_pool.enqueue(job);
    pthread_cond_signal(&eq);
    pthread_mutex_unlock(&qmtx);
  }

  void wait_all() {
    pthread_mutex_lock(&qmtx);
    while (job_pool.getLen() > 0 || busy > 0) {
      pthread_cond_wait(&jobs_done, &qmtx);
    }
    pthread_mutex_unlock(&qmtx);
  }
};

#endif