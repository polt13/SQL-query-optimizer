#ifndef PT_H
#define PT_H
#include "dataForm.h"
#include "node.h"
#include <cstdio>

// basically a linked list containing all records with equal n final bits
class HistEntry {
  // length of the partition : number of tuples in it
  int64_t len;

  Node* start;
  Node* end;

 public:
  void append(tuple);

  Node* getHistEntries() const;

  // length of a partition, in terms of number of bytes
  int64_t getLen() const;

  // for testing
  void print() const;

  HistEntry();

  ~HistEntry();
};
#endif