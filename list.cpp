#include "list.h"

#include <cstdio>

const Node* List::getRoot() const { return start; }

int64_t List::getLen() const { return len; }

void List::append(int64_t row) {
  if (len == 0)
    start = end = new Node(row);
  else {
    // always maintain a pointer that points to the last node, so we can
    // immediately insert
    end->next = new Node(row);
    end = end->next;
  }
  len++;
}

void List::print() const {
  Node* traverse = start;
  for (int64_t i = 0; i < len; i++) {
    std::printf("RowID #%ld : %ld\n", i, traverse->rowID);
    traverse = traverse->next;
  }
}

List::~List() {
  Node* traverse = start;
  for (int i = 0; i < len; i++) {
    Node* t = traverse;
    traverse = traverse->next;
    delete t;
  }
}