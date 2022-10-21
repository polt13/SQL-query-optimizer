#include "../include/dataForm.h"
#include "../include/partition.h"
#include <cstdio>
#define USE_BITS 2       // first partitioning
#define USE_BITS_NEXT 4  // 2nd partitioning
#define L2_SIZE 496

class Histogram;

/*
 * Hash Function for partitioning
 * Get the n Least Significant Bits (LSB)
 *
 */
uint64_t hash1(uint64_t key, uint64_t n) {
  uint64_t num = 1;
  num <<= n;
  // e.g. n = 3
  // 1000 - 1 = 111
  // val = key & (2^n - 1); // bitwise AND
  return key & (num - 1);
}

class Histogram {
  // histogram = array of partitions
  Partition* h;
  uint64_t partitions;

 public:
  Histogram(uint64_t size) {
    // create histogram with 2^n entries -- zero initialize all
    // each row  is basically a partition
    h = new Partition[size];
    partitions = size;
  }

  const Partition& getPartition(int64_t partitionNumber) const {
    return h[partitionNumber];
  }

  // partition size in BYTES
  uint64_t getPartitionSize(int64_t partitionIndex) const {
    return h[partitionIndex].getSize();
  }

  uint64_t getPartitionCount() const { return partitions; }

  // insert a rowID to the right partition (index are the final n bits)
  void insert(int64_t index, tuple t) { h[index].append(t); }

  ~Histogram() {
    // cleanup the list later
  }
};

class Partitioner {
  Histogram* hist;
  uint64_t r_entries;

  void partition1(relation r) {
    r_entries = r.getAmount();

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

  void partition2(relation r) {
    bool partitionsFit = true;

    for (uint64_t i = 0; i < hist->getPartitionCount(); i++) {
      if (hist->getPartitionSize(i) > L2_SIZE) {
        partitionsFit = false;
        std::printf("Partition %ld doesn't fit in L2", i);
        break;
      }
    }

    if (partitionsFit == false) {
      // discard old histogram, create a new using n2
      Histogram* old = hist;
      hist = new Histogram(1 << USE_BITS_NEXT);
      // for every partition, for every record..
      for (int64_t p = 0; p < old->getPartitionCount(); p++) {
        const Partition& partition = old->getPartition(p);
        // repartition based on new BIT SET and insert to hist
        int64_t partitionRecords = partition.getLen();
        Node* traverse = partition.getPartitionList();
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

 public:
  // 2^n sized histogram
  Partitioner() : hist{new Histogram(1 << USE_BITS)} {}

  void partition(relation r) {
    partition1(r);
    partition2(r);
  }
  ~Partitioner() { delete hist; }
};
