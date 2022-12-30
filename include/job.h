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
  virtual ~Job() {}
};

class HistogramJob : public Job {
  // args
  relation& r;
  size_t start;
  size_t end;
  Histogram*& h;
  int64_t bits;

 public:
  void run() { calcHist(r, start, end, h, bits); }

  HistogramJob(relation& r, size_t start, size_t end, Histogram*& h,
               int64_t bits)
      : Job(0), r{r}, start{start}, end{end}, h{h}, bits{bits} {}
};

class PartitionJob : public Job {};

class JoinJob : public Job {
  relation& s;
  int64_t start;
  int64_t end;
  hashTable* partitionHT;
  result& result_join;

 public:
  void run() { joinBuckets(s, start, end, partitionHT, result_join); }
  JoinJob(relation& s, int64_t start, int64_t end, hashTable* partitionHT,
          result& result_join)
      : Job(1),
        s{s},
        start{start},
        end{end},
        partitionHT{partitionHT},
        result_join{result_join} {}
};

#endif