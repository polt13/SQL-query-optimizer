#ifndef JOB_H
#define JOB_H
#include "dataForm.h"
#include "partitioner.h"

class Job {
  short int job_type;  // 0 = HistogramJob, 1 = PartitionJob
 public:
  short int get_job_type() const { return job_type; }
  virtual void* run() = 0;
  Job(short int job_type) : job_type{job_type} {}
};

class HistogramJob : public Job {
  // cast to Histogram* later
  Histogram* (*calcHist)(relation&, size_t, size_t);
  // args
  relation r;
  size_t start;
  size_t stop;

 public:
  void* run() { return (void*)calcHist(r, start, stop); }

  HistogramJob(relation& r, size_t start, size_t stop,
               Histogram* (*fn)(relation&, size_t, size_t))
      : Job(0), calcHist{fn}, r{r}, start{start}, stop{stop} {}
};

class PartitionJob : public Job {};

#endif