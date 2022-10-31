#ifndef LIST_H
#define LIST_H
#include <cstdint>

#include "dataForm.h"

struct Node {
  tuple* mytuple;
  Node* next;

 public:
  Node(tuple*, Node*);
};

class List {
  Node* start;
  Node* end;
  int64_t len;

 public:
  // Getters
  Node* getRoot() const;
  int64_t getLen() const;

  void append(tuple*);
  bool find(tuple&);
  void print() const;

  List();
  ~List();
};

#endif