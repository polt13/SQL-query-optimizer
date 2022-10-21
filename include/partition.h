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

  Node* getPartitionList() const { return start; }

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
#endif