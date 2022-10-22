#ifndef NODE_H
#define NODE_H
#include "dataForm.h"

struct Node {
  tuple t;
  Node* next;

  Node(tuple t, Node* next = nullptr) {
    this->t = t;
    this->next = next;
  }
};
#endif