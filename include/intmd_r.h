#ifndef INTMD_R
#define INTMD_R
#include <cstdint>

struct Node {
  int rowID;
  Node* next;

 public:
  Node(int, Node*);
};

class LinkedList {
  Node* start;
  Node* end;
  int64_t len;

 public:
  // Getters
  Node* getRoot() const;
  int64_t getLen() const;

  void append(int);
  bool find(int);
  void print() const;

  List();
  ~List();
};

/* Intermediate Result */
class intmd_r {
  int64_t size;
  LinkedList *relations;

 public:
  intmd_r(int64_t);

  // Getters
  int64_t getSize() const;
  // Setters
};

#endif