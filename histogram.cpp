#include "histogram.h"

Histogram::Histogram(uint64_t size) {
  // create histogram with 2^n entries -- zero initialize all
  // each row  is basically a partition
  h = new Partition[size];
  partitions = size;
}

const Partition& Histogram::getPartition(int64_t partitionNumber) const {
  return h[partitionNumber];
}

// partition size in BYTES
uint64_t Histogram::getPartitionSize(int64_t partitionIndex) const {
  return h[partitionIndex].getSize();
}

uint64_t Histogram::getPartitionCount() const { return partitions; }

// insert a rowID to the right partition (index are the final n bits)
void Histogram::insert(int64_t index, tuple t) { h[index].append(t); }

Histogram::~Histogram() {
  // cleanup list later..
}