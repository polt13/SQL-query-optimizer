#ifndef PARTITIONER_H
#define PARTITIONER_H
#include "dataForm.h"
#include <cstdint>
#include "histogram.h"

#define USE_BITS 2       // first partitioning
#define USE_BITS_NEXT 4  // 2nd partitioning
#define L2_SIZE 2500

class Partitioner {
  Histogram* hist;
  // how many partition phases the R relation went through
  int64_t partitioningLevel;

  relation partition1(relation&, int64_t = USE_BITS);

  relation partition2(relation&, int64_t = USE_BITS_NEXT);

 public:
  static int64_t hash1(uint64_t, uint64_t);

  int64_t getPartitioningLevel() const;

  // 2^n sized histogram
  Partitioner();

  // optionally : force partition-1 pass or 2 passes, change bits used for
  // partitioning
  relation partition(relation&, int64_t = -1, int64_t = USE_BITS,
                     int64_t = USE_BITS_NEXT);

  Histogram* getHistogram() const;

  ~Partitioner();
};

/* Partitioned Hash Join */
result PartitionedHashJoin(relation&, relation&, int64_t = -1,
                           int64_t = USE_BITS, int64_t = USE_BITS_NEXT);

#endif