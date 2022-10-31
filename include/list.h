#ifndef LIST_H
#define LIST_H
#include <cstdint>

struct Node {
  int64_t rowID;
  Node* next;

 public:
  Node(int64_t row, Node* next = nullptr) {
    this->rowID = row;
    this->next = next;
  }
};

class List {
  Node* start;
  Node* end;
  int64_t len;

 public:
  // Getters
  const Node* getRoot() const;
  int64_t getLen() const;
  List();
  void append(int64_t);
  void print() const;

  ~List();
};

#endif