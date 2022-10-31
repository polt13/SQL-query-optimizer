#include "list.h"

#include <cstdio>

Node::Node(tuple* t, Node* n = nullptr) : mytuple{t}, next{n} {}

// ------------------------------------------------------------------

Node* List::getRoot() const { return start; }

int64_t List::getLen() const { return len; }

void List::append(tuple* t) {
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

bool List::find(tuple& t) {
  Node* traverse = start;
  while (traverse) {
    if (traverse->mytuple == &t) return true;
    traverse = traverse->next;
  }
  return false;
}

void List::print() const {
  Node* traverse = start;
  for (int64_t i = 0; i < len; i++) {
    std::printf("Key %ld with payload %ld\n", traverse->mytuple->getKey(),
                traverse->mytuple->getPayload());
    traverse = traverse->next;
  }
}

List::List() : start{nullptr}, end{nullptr}, len{} {}

List::~List() {
  Node* traverse = start;
  for (int i = 0; i < len; i++) {
    Node* t = traverse;
    traverse = traverse->next;
    delete t;
  }
}