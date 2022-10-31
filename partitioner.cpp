#include "partitioner.h"
#include <cstdio>
#include <cstring>

relation Partitioner::partition1(relation& r) {
  int64_t r_entries = r.getAmount();
  // if partitioning is needed at least once
  std::printf("\nOne pass needed\n");
  partitioningLevel = 1;

  // partition based on payload
  for (int64_t t = 0; t < r_entries; t++) {
    // index indicates which partition the tuple goes to
    // t is the value = row_id
    tuple record = r[t];
    // partition based on the payload
    int64_t index = hash1(record.getKey(), USE_BITS);
    (*hist)[index]++;
  }

  const int64_t* psum = hist->generatePsum();

  // create copy so that we know where the next item needs to go
  // psum_copy is being mutated
  int64_t* psum_copy = new int64_t[hist->getSize()];
  std::memmove(psum_copy, psum, hist->getSize() * sizeof(int64_t));

  // sorted r'
  relation r2(new tuple[r_entries], r_entries);

  for (int64_t t = 0; t < r_entries; t++) {
    int64_t index = hash1(r[t].getKey(), USE_BITS);
    int64_t insertTo = psum_copy[index];

    r2[insertTo] = r[t];
    // indicate that there's one less tuple to copy
    // move index forward
    psum_copy[index]++;
  }

  delete[] psum_copy;

  return r2;
}

relation Partitioner::partition2(relation& r2) {
  std::printf("Second pass needed\n");
  partitioningLevel = 2;

  int64_t r2_entries = r2.getAmount();

  // discard old histogram, create a new using n2
  Histogram* oldHist = hist;
  hist = new Histogram(1 << USE_BITS_NEXT);
  // do the same thing, using r2
  for (int64_t t = 0; t < r2_entries; t++) {
    tuple record = r2[t];
    int64_t index = hash1(record.getKey(), USE_BITS_NEXT);
    (*hist)[index]++;
  }

  const int64_t* psum = hist->generatePsum();

  int64_t* psum_copy = new int64_t[hist->getSize()];
  std::memmove(psum_copy, psum, hist->getSize() * sizeof(int64_t));

  relation r3(new tuple[r2_entries], r2_entries);

  for (int64_t t = 0; t < r2_entries; t++) {
    int64_t index = hash1(r2[t].getKey(), USE_BITS_NEXT);
    int64_t insertTo = psum_copy[index];
    r3[insertTo] = r2[t];
    psum_copy[index]++;
  }

  // delete histogram from pass 1
  delete oldHist;

  delete[] psum_copy;

  // if reordered once more, return final
  return r3;
}

relation Partitioner::partition(relation& r, int64_t force_partition_depth) {
  if (((r.getAmount() * sizeof(tuple)) < L2_SIZE) &&
      force_partition_depth == 0) {
    std::printf("Doesn't need partitioning\n");
    return r;
  }

  relation r2 = partition1(r);
  if (force_partition_depth == 1) return r2;

  bool partitionsFit = true;

  for (int64_t i = 0; i < hist->getSize(); i++) {
    if ((*hist)[i] * sizeof(tuple) > L2_SIZE) {
      partitionsFit = false;
      std::printf("\nPartition %ld doesn't fit in L2\n", i);
      break;
    }
  }

  if (force_partition_depth != 2 && partitionsFit == true) return r2;

  return partition2(r2);
}

/*
 * Hash Function for partitioning
 * Get the n Least Significant Bits (LSB)
 *
 */
int64_t Partitioner::hash1(uint64_t key, uint64_t n) {
  uint64_t num = 1;
  num <<= n;
  // e.g. n = 3
  // 1000 - 1 = 111
  // val = key & (2^n - 1); // bitwise AND
  return key & (num - 1);
}

// 2^n sized histogram
Partitioner::Partitioner()
    : hist{new Histogram(1 << USE_BITS)}, partitioningLevel{} {}

Partitioner::~Partitioner() { delete hist; }