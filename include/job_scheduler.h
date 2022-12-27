#ifndef JOBSCHD_H
#define JOBSCHD_H

#include "simple_queue.h"
#include <pthread.h>
#include "job.h"
#include "config.h"

class JobScheduler {
  size_t avail;
  simple_queue<pthread_t> thread_pool;

 public:
  JobScheduler() : avail{THREAD_COUNT} {}
};

#endif