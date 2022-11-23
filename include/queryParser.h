#ifndef QP
#define QP
#include <cstdio>
#include "simple_vector.h"

constexpr size_t relation_count = 14;

class QueryParser {
  size_t relations_index[relation_count];
  simple_vector<char*> parse_names(char*);

 public:
  void parse_query(char*);
};
#endif