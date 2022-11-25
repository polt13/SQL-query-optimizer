#ifndef QP
#define QP
#include <cstdio>

#include "simple_vector.h"

constexpr size_t relation_count = 14;

enum operators { EQ, GREATER, LESS };

struct operations {
  // store literal value OR a.b (relation.column) in string form
  char* left;
  operators op;
  char* right;

  operations() = default;

  operations(char* left, operators op, char* right)
      : left{left}, op{op}, right{right} {}
};

struct project_rel {
  long rel;
  long col;
};

class QueryExec {
  simple_vector<long int> rel_names;
  simple_vector<operations> predicates;
  simple_vector<project_rel> projections;

  void parse_names(char*);
  void parse_predicates(char*);
  void parse_selections(char*);

  // ignore return types & args
  void do_query();
  void checksum();

 public:
  void parse_query(char*);
  void execute(char*);
};

#endif