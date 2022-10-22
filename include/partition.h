#ifndef PT_H
#define PT_H
#include "dataForm.h"
#include <cstdio>

struct Node {
  tuple t;
  Node* next;

  Node(tuple t, Node* next = nullptr) {
    this->t = t;
    this->next = next;
  }
};

// basically a linked list containing all records with equal n final bits
class Partition {
  // length of the partition : number of tuples in it
  uint64_t len;

  Node* start;
  Node* end;

 public:
  void append(tuple);

  void find(int64_t) const;

  Node* getPartitionList() const;

  // length of a partition, in terms of number of bytes
  uint64_t getLen() const;

  // size of a partition in BYTES
  uint64_t getSize() const;

  Partition();
};
#endif