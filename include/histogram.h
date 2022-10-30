#ifndef HIST_H
#define HIST_H

#include <cstdint>

class Histogram {
  int64_t* entries;
  int64_t* psum;
  int64_t size;

 public:
  Histogram(int64_t);

  // insert a rowID to the right partition (index are the final n bits)
  int64_t& operator[](int64_t);

  int64_t getSize() const;

  int64_t getPartitionEntries(int64_t) const;

  int64_t getPartitionPsum(int64_t) const;

  relation generateSortedRelation();

  ~Histogram();

  int64_t* generatePsum();
};
#endif