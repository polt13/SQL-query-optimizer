#ifndef QP
#define QP
#include <cstdio>

#include "dataForm.h"
#include "map_info.h"
#include "simple_vector.h"

constexpr size_t relation_count = 14;

enum operators { EQ, GREATER, LESS };

struct filter {
  int64_t rel;
  int64_t col;
  operators op;
  int64_t literal;

  filter() = default;

  filter(int64_t lr, int64_t lc, operators o, int64_t lit)
      : rel{lr}, col{lc}, op{o}, literal{lit} {}
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
  int64_t rel;
  int64_t col;
};

class QueryExec {
  simple_vector<int64_t> rel_names;
  simple_vector<join> joins;
  simple_vector<filter> filters;
  simple_vector<project_rel> projections;
  simple_vector<bool> used_relations;

  void parse_query(char*);
  void parse_names(char*);
  void parse_predicates(char*);
  void parse_selections(char*);

  void do_query();

  void filter_exec(size_t, simple_vector<int64_t>&);
  simple_vector<result_item> do_self_join(simple_vector<int64_t>&, int64_t);
  simple_vector<result_item> do_simple_join(simple_vector<int64_t>&,
                                            simple_vector<int64_t>&, int64_t);
  result do_hash_join(simple_vector<int64_t>&, simple_vector<int64_t>&,
                      int64_t);

  void checksum(simple_vector<int64_t>[]);
  void clear();

 public:
  void execute(char*);
};

#endif