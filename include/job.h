#ifndef JOB_H
#define JOB_H
#include "dataForm.h"
#include "partitioner.h"

class Job {
 public:
  virtual void run() = 0;
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
      : r{r}, start{start}, end{end}, h{h}, bits{bits} {}
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
      : s{s},
        start{start},
        end{end},
        partitionHT{partitionHT},
        result_join{result_join} {}
};

class BuildJob : public Job {
  relation& r;
  int64_t start;
  int64_t end;
  hashTable* partitionHT;

 public:
  void run() { buildHT(r, start, end, partitionHT); }

  BuildJob(relation& r, int64_t start, int64_t end, hashTable* partitionHT)
      : r{r}, start{start}, end{end}, partitionHT{partitionHT} {}
};

#endif
