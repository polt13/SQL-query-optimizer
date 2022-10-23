#include "partition.h"

// append new rowID at the end of the lis
void Partition::append(tuple t) {
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

void Partition::find(int64_t key) const {
  Node* traverse = start;
  while (traverse) {
    if (traverse->t.getKey() == key) {
      std::printf("Found item with key %ld", key);
      break;
    }
    traverse = traverse->next;
  }
}

Node* Partition::getPartitionList() const { return start; }

// length of a partition, in terms of number of bytes
uint64_t Partition::getLen() const { return len; }

// size of a partition in BYTES
uint64_t Partition::getSize() const {
  // linked list entries * size of node
  return sizeof(Node) * len;
}

void Partition::print() const {
  Node* traverse = start;
  for (int64_t i = 0; i < len; i++) {
    std::printf("tuple key : %ld, payload : %ld\n", traverse->t.getKey(),
                traverse->t.getPayload());
    traverse = traverse->next;
  }
}

Partition::Partition() {
  start = end = nullptr;
  len = 0;
}

Partition::~Partition() {
  Node* traverse = start;
  for (int i = 0; i < len; i++) {
    Node* t = traverse;
    traverse = traverse->next;
    delete t;
  }
}
