#include "../include/interface.h"
#include <cstdio>
#define USE_BITS 2       // first partitioning
#define USE_BITS_NEXT 4  // 2nd partitioning
#define L2_SIZE 496

class Histogram;

// basically a linked list containing all records with equal n final bits
class Partition {
  struct Node {
    tuple t;
    Node* next;

    Node(tuple t, Node* next = nullptr) {
      this->t = t;
      this->next = next;
    }
  };

  // length of the partition : number of tuples in it
  uint64_t len;

  Node* start;
  Node* end;

 public:
  // append new rowID at the end of the list
  void append(tuple t) {
    if (len == 0)
      start = end = new Node(t);
    else {
      // always maintain a pointer that points to the last node, so we can
      // immediately insert
      end->next = new Node(t);
      end = end->next;
    }
    len++;
  }

  void find(int64_t key) const {
    Node* traverse = start;
    while (traverse) {
      if (traverse->t.getKey() == key) {
        std::printf("Found item with key %ld", key);
        break;
      }
      traverse = traverse->next;
    }
  }

  // length of a partition, in terms of number of bytes
  uint64_t getLen() const { return len; }

  // size of a partition in BYTES
  uint64_t getSize() const {
    // linked list entries * size of node
    return sizeof(Node) * len;
  }

  Partition() {
    start = end = nullptr;
    len = 0;
  }
};

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

  // 2^n sized histogram
  Partitioner() : hist{new Histogram(1 << USE_BITS)} {}

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

  void partition1(relation r) {
    r_entries = r.getAmount();

    // partition based on payload
    for (int64_t t = 0; t < r_entries; t++) {
      // index indicates which partition the tuple goes to
      // t is the value = row_id
      int64_t index = hash1(r.getTuple(t).getPayload(), USE_BITS);
      hist->insert(index, r.getTuple(t));
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
        int64_t partitionRecords = partition.getLen();
        // partition the partitions and insert the entries in new histogram
        for (int64_t t = 0; t < partitionRecords; t++) {
          // TRAVERSE RECORDS IN PARTITION AND INSERT THEM BACK
          int64_t index = hash1(traverse->t.getPayload(), USE_BITS_NEXT);
          hist->insert(index, traverse->t);
          traverse = traverse->next;
        }
      }

      // delete histogram from pass 1
      delete old;
    }
  }

  ~Partitioner() { delete hist; }
};
