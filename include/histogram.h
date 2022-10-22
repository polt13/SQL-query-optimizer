#ifndef HIST_H
#define HIST_H
#include "partition.h"
#include <cstdint>

class Histogram {
  // histogram = array of partitions
  Partition* h;
  uint64_t partitions;

 public:
  Histogram(uint64_t size);

  const Partition& getPartition(int64_t) const;

  // partition size in BYTES
  uint64_t getPartitionSize(int64_t) const;

  uint64_t getPartitionCount() const;

  // insert a rowID to the right partition (index are the final n bits)
  void insert(int64_t, tuple);

  ~Histogram();
};

#endif