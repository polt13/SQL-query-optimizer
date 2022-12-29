#ifndef HIST_H
#define HIST_H

#include <cstdint>
#include "dataForm.h"

class Histogram {
  int64_t* entries;
  int64_t* psum;
  int64_t size;

 public:
  Histogram(int64_t);

  int64_t& operator[](int64_t);

  int64_t getSize() const;

  int64_t getPartitionPsum(int64_t) const;

  ~Histogram();

  const int64_t* generatePsum();

  // for later use in the hashtable insertion and traversing
  const int64_t* getPsum() const { return psum; }
};

void calcHist(relation&, size_t, size_t, Histogram&, int64_t);

#endif