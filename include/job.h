#ifndef JOB_H
#define JOB_H
#include "dataForm.h"
#include "partitioner.h"

class Job {
  short int job_type;  // 0 = HistogramJob, 1 = PartitionJob
 public:
  short int get_job_type() const { return job_type; }
  virtual void run() = 0;
  Job(short int job_type) : job_type{job_type} {}
};

class HistogramJob : public Job {
  // args
  relation& r;
  size_t start;
  size_t end;
  Histogram& h;
  int64_t bits;

 public:
  void run() { calcHist(r, start, end, h, bits); }

  HistogramJob(relation& r, size_t start, size_t end, Histogram& h,
               int64_t bits)
      : Job(0), r{r}, start{start}, end{end}, h(h) {}
};

class PartitionJob : public Job {};

#endif