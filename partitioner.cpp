#include "partitioner.h"
#include <cstdio>

relation Partitioner::partition1(relation r) {
  int64_t r_entries = r.getAmount();

  // change maybe? (sizeof(relation) = 8, if it only holds a pointer it always
  // fits)
  if (r.getAmount() * sizeof(tuple) < L2_SIZE) {
    std::printf("Doesn't need partitioning\n");
    return;
  }

  // if partitioning is needed at least once
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

  int64_t* psum = hist->generatePsum();
  // sorted r'
  relation r2(new tuple[r_entries], r_entries);

  for (int64_t t = 0; t < r_entries; t++) {
    int64_t index = hash1(r2[t].getKey(), USE_BITS);
    int64_t insertTo = psum[index];
    r2[insertTo] = r2[t];
    // indicate that there's one less tuple to copy
    // move index forward
    psum[index]++;
  }

  return r2;
}

relation Partitioner::partition2(relation r2) {
  bool partitionsFit = true;
  int64_t r2_entries = r2.getAmount();

  for (int64_t i = 0; i < hist->getSize(); i++) {
    if (hist->getPartitionEntries(i) * sizeof(tuple) > L2_SIZE) {
      partitionsFit = false;
      std::printf("\nPartition %ld doesn't fit in L2\n", i);
      break;
    }
  }

  if (partitionsFit == false) {
    partitioningLevel = 2;
    std::printf("Second pass needed\n");
    // discard old histogram, create a new using n2
    Histogram* old = hist;
    hist = new Histogram(1 << USE_BITS_NEXT);
    // do the same thing, using r2
    for (int64_t t = 0; t < r2_entries; t++) {
      // index indicates which partition the tuple goes to
      // t is the value = row_id
      tuple record = r2[t];
      // partition based on the payload
      int64_t index = hash1(record.getKey(), USE_BITS);
      (*hist)[index]++;
    }

    int64_t* psum = hist->generatePsum();
    // sorted r'
    relation r3(new tuple[r2_entries], r2_entries);

    for (int64_t t = 0; t < r2_entries; t++) {
      int64_t index = hash1(r2[t].getKey(), USE_BITS);
      int64_t insertTo = psum[index];
      r3[insertTo] = r2[t];
      // indicate that there's one less tuple to copy
      // move index forward
      psum[index]++;
    }

    // delete histogram from pass 1
    delete old;
    // if reordered once more, return final
    return r3;
  }

  // if it fits after one partitioning, return the previous
  return r2;
}

void Partitioner::partition(relation r) {
  relation r2 = partition1(r);
  relation r_final = partition2(r2);
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

// void Partitioner::printEntries() const {
//   for (int64_t i = 0; i < hist->getEntriesCount(); i++) {
//     std::printf("\nPartition %ld\n", i);
//     hist->getEntry(i).print();
//   }
// }

// 2^n sized histogram
Partitioner::Partitioner()
    : hist{new Histogram(1 << USE_BITS)}, partitioningLevel{} {}

Partitioner::~Partitioner() { delete hist; }