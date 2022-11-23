#ifndef QP
#define QP
#include <cstdio>
#include "simple_vector.h"

constexpr size_t relation_count = 14;

enum class operators { EQ, GREATER, LESS };

struct operations {
  // store literal value OR a.b (relation.column) in string form
  char* left;
  operators op;
  char* right;
};

class QueryParser {
  size_t relations_index[relation_count];
  simple_vector<char*> parse_names(char*);
  simple_vector<operations> parse_predicates(char*);

 public:
  void parse_query(char*);
};

#endif