#ifndef JOBSCHD_H
#define JOBSCHD_H

#include "simple_queue.h"
#include <pthread.h>
#include "job.h"
#include "config.h"

class JobScheduler {
  pthread_t* total_threads;
  pthread_mutex_t qmtx;
  pthread_cond_t eq;
  pthread_cond_t jobs_done;
  simple_queue<Job*> job_pool;
  int busy;

  static void* do_job(void* _this) {
    JobScheduler* obj = (JobScheduler*)_this;
    while (1) {
      pthread_mutex_lock(&obj->qmtx);

      while (obj->job_pool.getLen() == 0) {
        pthread_cond_wait(&obj->eq, &obj->qmtx);
      }

      Job* j = obj->job_pool.pop();

      ++obj->busy;

      pthread_mutex_unlock(&obj->qmtx);

      j->run();

      pthread_mutex_lock(&obj->qmtx);
      --obj->busy;
      if ((obj->busy == 0) && (obj->job_pool.getLen() == 0))
        pthread_cond_broadcast(&obj->jobs_done);
      pthread_mutex_unlock(&obj->qmtx);

      delete j;
    }
  }

 public:
  JobScheduler(int threads) {
    total_threads = new pthread_t[threads];
    busy = 0;
    pthread_cond_init(&eq, nullptr);
    pthread_cond_init(&jobs_done, nullptr);
    pthread_mutex_init(&qmtx, nullptr);
    for (int i = 0; i < threads; i++)
      pthread_create(total_threads + i, nullptr, do_job, this);
  }

  void add_job(Job* job) {
    pthread_mutex_lock(&qmtx);
    job_pool.enqueue(job);
    pthread_cond_signal(&eq);
    pthread_mutex_unlock(&qmtx);
  }

  ~JobScheduler() { delete[] total_threads; }

  void wait_all() {
    pthread_mutex_lock(&qmtx);
    while (job_pool.getLen() > 0 || busy > 0) {
      pthread_cond_wait(&jobs_done, &qmtx);
    }
    pthread_mutex_unlock(&qmtx);
  }
};

#endif