#include "partitioner.h"

void Partitioner::partition1(relation r) {
  int64_t r_entries = r.getAmount();

  // partition based on payload
  for (int64_t t = 0; t < r_entries; t++) {
    // index indicates which partition the tuple goes to
    // t is the value = row_id
    tuple record = r.getTuple(t);
    // partition based on the payload
    int64_t index = hash1(record.getPayload(), USE_BITS);
    hist->insert(index, record);
  }
}

void Partitioner::partition2() {
  bool partitionsFit = true;

  for (int64_t i = 0; i < hist->getEntriesCount(); i++) {
    if (hist->getPartitionSize(i) > L2_SIZE) {
      partitionsFit = false;
      std::printf("\nPartition %ld doesn't fit in L2\n", i);
      break;
    }
  }

  if (partitionsFit == false) {
    std::printf("Second pass needed\n");
    // discard old histogram, create a new using n2
    Histogram* old = hist;
    hist = new Histogram(1 << USE_BITS_NEXT);
    // for every partition, for every record..
    for (int64_t p = 0; p < old->getEntriesCount(); p++) {
      const HistEntry& partition = old->getEntry(p);
      // repartition based on new BIT SET and insert to hist
      int64_t partitionRecords = partition.getLen();
      Node* traverse = partition.getHistEntries();
      for (int64_t r = 0; r < partitionRecords; r++) {
        tuple record = traverse->t;
        int64_t new_index = hash1(record.getPayload(), USE_BITS_NEXT);
        hist->insert(new_index, record);
        traverse = traverse->next;
      }
    }
    // delete histogram from pass 1
    delete old;
  }
}

void Partitioner::partition(relation r) {
  partition1(r);
  partition2();
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

void Partitioner::printEntries() const {
  for (int64_t i = 0; i < hist->getEntriesCount(); i++) {
    std::printf("\nPartition %ld\n", i);
    hist->getEntry(i).print();
  }
}

// 2^n sized histogram
Partitioner::Partitioner() : hist{new Histogram(1 << USE_BITS)} {}

Partitioner::~Partitioner() { delete hist; }