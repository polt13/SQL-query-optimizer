#ifndef QP
#define QP
#include <cstdio>

#include "map_info.h"
#include "dataForm.h"
#include "simple_vector.h"

constexpr size_t relation_count = 14;

enum operators { EQ, GREATER, LESS };

struct filter {
  int64_t left_rel;
  int64_t left_col;
  operators op;
  int64_t literal;

  filter() = default;

  filter(int64_t lr, int64_t lc, operators o, int64_t lit)
      : left_rel{lr}, left_col{lc}, op{o}, literal{lit} {}
};

struct join {
  int64_t left_rel;
  int64_t left_col;
  operators op;
  int64_t right_rel;
  int64_t right_col;

  join() = default;

  join(int64_t lr, int64_t lc, operators o, int64_t rr, int64_t rc)
      : left_rel{lr}, left_col{lc}, op{o}, right_rel{rr}, right_col{rc} {}
};

struct project_rel {
  long rel;
  long col;
};

class QueryExec {
  simple_vector<long int> rel_names;
  simple_vector<join> joins;
  simple_vector<filter> filters;
  simple_vector<project_rel> projections;

  void parse_query(char*);

  void parse_names(char*);
  void parse_predicates(char*);
  void parse_selections(char*);

  void clear();

  simple_vector<result_item> self_join(simple_vector<long int>&);

  // ignore return types & args
  void do_query();
  void checksum();

 public:
  void execute(char*);
};

#endif