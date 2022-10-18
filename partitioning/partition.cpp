#include "../include/interface.h"
#include <iostream>
#define USE_BITS 4

class LinkedList {
  uint64_t size;
  Node* start;

  struct Node {
    uint32_t elem;
    Node* next;

    Node(uint32_t elem) { this->elem = elem; }
  }

  public : void
           insert(uint32_t element) {
    Node** traverse = &start;
    while (*traverse) traverse = &((*traverse)->next);
    *traverse = new Node(element);
    size++;
  }

  void find(uint32_t element) {
    Node* traverse = start;
    while (traverse) {
      if (traverse->elem == element) {
        std::cout << "found\n";
        break;
      }
      traverse = traverse->next;
    }
  }

  uint32_t getSize() { return size; }

  LinkedList() {
    start = nullptr;
    size = 0;
  }

}

class Histogram {
  LinkedList* h;

 public:
  Histogram(uint64_t size) {
    // create histogram with 2^n entries -- zero initialize all
    // each row  is basically a partition
    h = new LinkedList[size];
    this->size = size;
  }

  void insert(uint32_t index, uint32_t val) { h[index].insert(val) }

  ~Histogram() {
    // cleanup the list later
  }
};

class Partitioner {
  Histogram hist;

  // 2^n sized histogram
  Partitioner() : hist{1 << USE_BITS} {}

  /*
   * Hash Function for partitioning
   * Get the n Least Significant Bits (LSB)
   *
   */
  uint32_t hash1(uint32_t key, uint32_t n) {
    uint32_t num = 1;
    num <<= n;
    // e.g. n = 3
    // 1000 - 1 = 111
    // val = key & (2^n - 1); // bitwise AND
    return key & (num - 1);
  }

  void partition(relation r) {
    uint32_t r_entries = r.getAmount();

    relation temp(r_entries);

    // partition based on payload
    for (int t = 0; t < r_entries; t++) {
      // index indicates which partition the tuple goes to
      // t is the value = row_id
      uint64_t index = hash1(r.getTuple(t).getPayload(), USE_BITS);
      hist.insert(index, t)
    }
  }
};
