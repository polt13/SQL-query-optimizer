#ifndef PARTITIONER_H
#define PARTITIONER_H
#include "dataForm.h"
#include <cstdint>
#include "histogram.h"

#define USE_BITS 2       // first partitioning
#define USE_BITS_NEXT 4  // 2nd partitioning
#define L2_SIZE 600

class Partitioner {
  Histogram* hist;
  // how many partition phases the R relation went through
  int64_t partitioningLevel;

  relation partition1(relation&);

  relation partition2(relation&);

 public:
  static int64_t hash1(uint64_t, uint64_t);

  int64_t getPartitioningLevel() const;

  // 2^n sized histogram
  Partitioner();

  relation partition(relation&, int64_t = -1);

  Histogram* getHistogram() const;

  ~Partitioner();
};
#endif