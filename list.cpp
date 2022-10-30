#include "list.h"
#include <cstdio>
void List::append(tuple t) {
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

List::~List() {
  Node* traverse = start;
  for (int i = 0; i < len; i++) {
    Node* t = traverse;
    traverse = traverse->next;
    delete t;
  }
}

int64_t List::getLen() const { return len; }

const Node* List::getRoot() const { return start; }

void List::print() const {
  Node* traverse = start;
  for (int64_t i = 0; i < len; i++) {
    std::printf("tuple key : %ld, payload : %ld\n", traverse->t.getKey(),
                traverse->t.getPayload());
    traverse = traverse->next;
  }
}