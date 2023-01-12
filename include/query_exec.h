#ifndef QP
#define QP
#include <cstdio>

#include "dataForm.h"
#include "map_info.h"
#include "simple_vector.h"

enum operators { EQ, GREATER, LESS };

struct filter {
  int32_t rel;
  int32_t col;
  operators op;
  int32_t literal;

  filter() = default;

  filter(int32_t lr, int32_t lc, operators o, int32_t lit)
      : rel{lr}, col{lc}, op{o}, literal{lit} {}
};

struct join {
  int32_t left_rel;
  int32_t left_col;
  operators op;
  int32_t right_rel;
  int32_t right_col;

  join() = default;

  join(int32_t lr, int32_t lc, operators o, int32_t rr, int32_t rc)
      : left_rel{lr}, left_col{lc}, op{o}, right_rel{rr}, right_col{rc} {}
};

struct project_rel {
  int32_t rel;
  int32_t col;
};

class QueryExec {
  int32_t qindex;  // which query
  simple_vector<int32_t> rel_names;
  simple_vector<join> joins;
  simple_vector<filter> filters;
  simple_vector<project_rel> projections;

  bool rel_is_filtered[4];
  bool rel_is_joined[4];

  simple_vector<int32_t>
      joined[4];  // Represents latest int32_termediate results
  simple_vector<int32_t> filtered[4];

  statistics *rel_stats[4];

  void parse_query(char *);
  void parse_names(char *);
  void parse_predicates(char *);
  void parse_selections(char *);

  void initialize_stats();
  void join_enumeration();
  bool isConnected(simple_vector<join> &, size_t);
  uint64_t calculate_cost(size_t);
  void update_stats(size_t, int32_t);

  void do_query();
  void filter_exec(size_t);
  void do_join(size_t);

  void checksum();

 public:
  QueryExec(int32_t);
  void execute(char *);
};

#endif