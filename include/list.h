#ifndef LIST_H
#define LIST_H
#include "dataForm.h"
#include <cstdint>

struct Node {
  tuple t;
  Node* next;

  Node(tuple t, Node* next = nullptr) {
    this->t = t;
    this->next = next;
  }
};

class List {
  Node* start;
  Node* end;
  int64_t len;

 public:
  int64_t getLen() const;
  void append(tuple);
  void print() const;
  const Node* getRoot() const;
  ~List();
};

#endif