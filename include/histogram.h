#ifndef HIST_H
#define HIST_H
#include "partition.h"
#include <cstdint>

class Histogram {
  // histogram = array of partitions
  HistEntry* h;
  uint64_t entries;

 public:
  Histogram(int64_t);

  const HistEntry& getEntry(int64_t) const;

  int64_t getEntriesCount() const;

  // insert a rowID to the right partition (index are the final n bits)
  void insert(int64_t, tuple);

  ~Histogram();
};

#endif