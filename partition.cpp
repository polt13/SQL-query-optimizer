#include "partition.h"

// append new rowID at the end of the lis
void HistEntry::append(tuple t) {
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

Node* HistEntry::getHistEntries() const { return start; }

// length of a partition, in terms of number of bytes
int64_t HistEntry::getLen() const { return len; }

void HistEntry::print() const {
  Node* traverse = start;
  for (int64_t i = 0; i < len; i++) {
    std::printf("tuple key : %ld, payload : %ld\n", traverse->t.getKey(),
                traverse->t.getPayload());
    traverse = traverse->next;
  }
}

HistEntry::HistEntry() {
  start = end = nullptr;
  len = 0;
}

HistEntry::~HistEntry() {
  Node* traverse = start;
  for (int i = 0; i < len; i++) {
    Node* t = traverse;
    traverse = traverse->next;
    delete t;
  }
}
