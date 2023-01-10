#ifndef QP
#define QP
#include <cstdio>

#include "dataForm.h"
#include "map_info.h"
#include "simple_vector.h"

enum operators { EQ, GREATER, LESS };

struct filter {
  int rel;
  int col;
  operators op;
  int64_t literal;

  filter() = default;

  filter(int lr, int lc, operators o, int64_t lit)
      : rel{lr}, col{lc}, op{o}, literal{lit} {}
};

struct join {
  int left_rel;
  int left_col;
  operators op;
  int right_rel;
  int right_col;

  join() = default;

  join(int lr, int lc, operators o, int rr, int rc)
      : left_rel{lr}, left_col{lc}, op{o}, right_rel{rr}, right_col{rc} {}
};

struct project_rel {
  int rel;
  int col;
};

class QueryExec {
  int qindex;  // which query
  simple_vector<int> rel_names;
  simple_vector<join> joins;
  simple_vector<filter> filters;
  simple_vector<project_rel> projections;

  bool rel_is_filtered[4];
  bool rel_is_joined[4];

  simple_vector<int> joined[4];  // Represents latest intermediate results
  simple_vector<int> filtered[4];

  statistics *rel_stats[4];

  void parse_query(char *);
  void parse_names(char *);
  void parse_predicates(char *);
  void parse_selections(char *);

  void initialize_stats();
  void join_enumeration();
  bool isConnected(simple_vector<join> &, size_t);
  uint64_t calculate_cost(size_t);
  void update_stats(size_t, int);

  void do_query();
  void filter_exec(size_t);
  void do_join(size_t);

  void checksum();

 public:
  QueryExec(int);
  void execute(char *);
};

#endif