#include "query_exec.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dataForm.h"
#include "map_info.h"
#include "partitioner.h"
#include "query_results.h"
#include "simple_vector.h"

extern QueryResults qres[100];

QueryExec::QueryExec(int32_t qindex)
    : qindex{qindex}, rel_names{4}, projections{4} {}

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
#ifdef Q_OPT
  initialize_stats();
#endif
  do_query();
  checksum();
  std::free(query);  // free strdup'd memory
}

//-----------------------------------------------------------------------------------------

void QueryExec::parse_query(char* query) {
  char* buffr;

  char* used_relations = strtok_r(query, "|", &buffr);
  parse_names(used_relations);

  char* predicates = strtok_r(nullptr, "|", &buffr);
  parse_predicates(predicates);

  // buffr now points to the last part of the query
  char* selections = buffr;
  parse_selections(selections);
}

//-----------------------------------------------------------------------------------------

void QueryExec::parse_names(char* rel_string) {
  char* buffr;
  char* rel;

  while ((rel = strtok_r(rel_string, " ", &buffr))) {
    this->rel_names.add_back(std::strtol(rel, nullptr, 10));
    rel_string = nullptr;
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::parse_predicates(char* predicates) {
  char *buffr, *buffr2, *buffr3;
  const char* op_val;

  char* predicate;

  operators operation_type;

  while ((predicate = strtok_r(predicates, "&", &buffr))) {
    if (std::strchr(predicate, '=')) {
      operation_type = operators::EQ;
      op_val = "=";
    } else if (std::strchr(predicate, '>')) {
      operation_type = operators::GREATER;
      op_val = ">";
    } else {
      operation_type = operators::LESS;
      op_val = "<";
    }

    char* left = strtok_r(predicate, op_val, &buffr2);

    if (std::strchr(left, '.') == nullptr) {
      // left contains literal - 100% filter
      int32_t literal = (int32_t)std::strtol(left, nullptr, 10);

      if (operation_type == operators::GREATER)
        operation_type = operators::LESS;
      else if (operation_type == operators::LESS)
        operation_type = operators::GREATER;
      else
        operation_type = operators::EQ;

      char* right = buffr2;

      int32_t right_rel =
          (int32_t)std::strtol(strtok_r(right, ".", &buffr3), nullptr, 10);

      int32_t right_col = (int32_t)std::strtol(buffr3, nullptr, 10);

      filter myfilter(right_rel, right_col, operation_type, literal);
      this->filters.add_back(myfilter);

    } else {
      // e.g 0.2 (relation.column) - Could be filter OR join
      int32_t left_rel =
          (int32_t)std::strtol(strtok_r(left, ".", &buffr3), nullptr, 10);
      int32_t left_col = (int32_t)std::strtol(buffr3, nullptr, 10);

      char* right = buffr2;
      if (std::strchr(right, '.') == nullptr) {
        // right contains literal - 100% filter
        int32_t literal = (int32_t)std::strtol(right, nullptr, 10);

        filter myfilter(left_rel, left_col, operation_type, literal);
        this->filters.add_back(myfilter);
      } else {
        // e.g 1.4 (relation.column) - 100% join (but can be same relation!!!)
        int32_t right_rel =
            (int32_t)std::strtol(strtok_r(right, ".", &buffr3), nullptr, 10);
        int32_t right_col = (int32_t)std::strtol(buffr3, nullptr, 10);

        join myjoin(left_rel, left_col, operation_type, right_rel, right_col);
        this->joins.add_back(myjoin);
      }
    }
    predicates = nullptr;
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::parse_selections(char* selections) {
  char *buffr, *buffr2;
  char* selection;

  while ((selection = strtok_r(selections, " ", &buffr))) {
    char* rel = strtok_r(selection, ".", &buffr2);
    char* col = buffr2;

    this->projections.add_back(
        project_rel{(int32_t)std::strtol(rel, nullptr, 10),
                    (int32_t)std::strtol(col, nullptr, 10)});

    selections = nullptr;
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::initialize_stats() {
  for (size_t i = 0; i < rel_names.getSize(); i++) {
    int32_t actual_rel = this->rel_names[i];
    this->rel_stats[i] = new statistics[rel_mmap[actual_rel].cols];
    for (size_t j = 0; j < rel_mmap[actual_rel].cols; j++) {
      this->rel_stats[i][j].l = rel_mmap[actual_rel].stats[j].l;
      this->rel_stats[i][j].u = rel_mmap[actual_rel].stats[j].u;
      this->rel_stats[i][j].f = rel_mmap[actual_rel].stats[j].f;
      this->rel_stats[i][j].d = rel_mmap[actual_rel].stats[j].d;
    }
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::join_enumeration() {
  size_t initial_size = joins.getSize();
  simple_vector<join> joins_order;

  while (joins.getSize() > 0) {
    uint64_t min_cost = UINT64_MAX;
    size_t min_index = SIZE_MAX;
    for (size_t i = 0; i < joins.getSize(); i++) {
      if ((joins.getSize() != initial_size) && (!isConnected(joins_order, i)))
        continue;
      uint64_t curr_cost = calculate_cost(i);
      if (curr_cost < min_cost) {
        min_cost = curr_cost;
        min_index = i;
      }
    }
    if (min_index == SIZE_MAX) exit(EXIT_FAILURE);
    update_stats(min_index, 1);
    joins_order.add_back(joins[min_index]);
    joins.remove(min_index);
  }
  joins.steal(joins_order);
}

//-----------------------------------------------------------------------------------------

bool QueryExec::isConnected(simple_vector<join>& joins_order,
                            size_t joins_index) {
  if (joins_order.getSize() == 0) return false;
  for (size_t i = 0; i < joins_order.getSize(); i++) {
    if (joins[joins_index].left_rel == joins_order[i].left_rel ||
        joins[joins_index].left_rel == joins_order[i].right_rel ||
        joins[joins_index].right_rel == joins_order[i].left_rel ||
        joins[joins_index].right_rel == joins_order[i].right_rel)
      return true;
  }
  return false;
}

//-----------------------------------------------------------------------------------------

uint64_t QueryExec::calculate_cost(size_t index) {
  int32_t r_rel = this->joins[index].left_rel;
  int32_t r_col = this->joins[index].left_col;
  int32_t s_rel = this->joins[index].right_rel;
  int32_t s_col = this->joins[index].right_col;
  int32_t actual_r = this->rel_names[r_rel];
  int32_t actual_s = this->rel_names[s_rel];
  uint64_t numerator, denominator, res;

  // Self-Join (e.g 0 0 | 0.1=1.1...)
  if ((actual_r == actual_s) && (r_col == s_col)) {
    numerator = rel_stats[r_rel][r_col].f * rel_stats[r_rel][r_col].f;
    denominator = rel_stats[r_rel][r_col].u - rel_stats[r_rel][r_col].l + 1;
    res = numerator / denominator;
    return res;
  }

  // Join between 2 different relations
  else {
    uint64_t lower = rel_stats[r_rel][r_col].l;
    uint64_t upper = rel_stats[r_rel][r_col].u;

    if (rel_stats[s_rel][s_col].l > lower) lower = rel_stats[s_rel][s_col].l;
    if (rel_stats[s_rel][s_col].u < upper) upper = rel_stats[s_rel][s_col].u;

    numerator = rel_stats[r_rel][r_col].f * rel_stats[s_rel][s_col].f;
    denominator = upper - lower + 1;
    res = numerator / denominator;
    return res;
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::update_stats(size_t index, int32_t flag) {
  if (flag == 0) {
    int32_t rel = this->filters[index].rel;
    int32_t actual_rel = this->rel_names[rel];
    int32_t col = this->filters[index].col;
    uint64_t lit = this->filters[index].literal;

    if (filtered[rel].getSize() == 0 || rel_stats[rel][col].f == 0 ||
        rel_stats[rel][col].d == 0) {
      return;
    }

    // Filter σ_A=k
    if (this->filters[index].op == operators::EQ) {
      uint64_t prev_f = this->rel_stats[rel][col].f;
      uint64_t prev_d = this->rel_stats[rel][col].d;

      this->rel_stats[rel][col].l = lit;
      this->rel_stats[rel][col].u = lit;

      this->rel_stats[rel][col].d = 0;
      this->rel_stats[rel][col].f = 0;
      for (uint64_t i = 0; i < rel_mmap[actual_rel].rows; i++)
        if (rel_mmap[actual_rel].colptr[col][i] == lit) {
          this->rel_stats[rel][col].d = 1;
          rel_stats[rel][col].f = prev_f / prev_d;
          break;
        }

      for (size_t i = 0; i < rel_mmap[actual_rel].cols; i++)
        if ((int32_t)i != col) {
          double base = (1 - ((double)rel_stats[rel][col].f / prev_f));
          double power = ((double)rel_stats[rel][i].f / rel_stats[rel][i].d);
          double res = pow(base, power);
          res = rel_stats[rel][i].d * (1 - res);
          rel_stats[rel][i].d = (uint64_t)res;

          rel_stats[rel][i].f = rel_stats[rel][col].f;
        }
    }

    // Filter σ_A>k OR σ_A<k
    else {
      uint64_t prev_l = rel_stats[rel][col].l;
      uint64_t prev_u = rel_stats[rel][col].u;
      uint64_t prev_f = rel_stats[rel][col].f;

      // Filter σ_A>k
      if (this->filters[index].op == operators::GREATER) {
        if (lit < rel_stats[rel][col].l) lit = rel_stats[rel][col].l;
        rel_stats[rel][col].l = lit;
      }

      // Filter σ_A<k
      if (this->filters[index].op == operators::LESS) {
        if (lit > rel_stats[rel][col].u) lit = rel_stats[rel][col].u;
        rel_stats[rel][col].u = lit;
      }

      rel_stats[rel][col].d *=
          (((double)rel_stats[rel][col].u - rel_stats[rel][col].l) /
           ((double)prev_u - prev_l));
      rel_stats[rel][col].f *=
          (((double)rel_stats[rel][col].u - rel_stats[rel][col].l) /
           ((double)prev_u - prev_l));

      for (size_t i = 0; i < rel_mmap[actual_rel].cols; i++)
        if ((int32_t)i != col) {
          double base = (1 - ((double)rel_stats[rel][col].f / prev_f));
          double power = ((double)rel_stats[rel][i].f / rel_stats[rel][i].d);
          double res = pow(base, power);
          res = rel_stats[rel][i].d * (1 - res);
          rel_stats[rel][i].d = (uint64_t)res;

          rel_stats[rel][i].f = rel_stats[rel][col].f;
        }
    }
  }
  if (flag == 1) {
    int32_t r_rel = this->joins[index].left_rel;
    int32_t r_col = this->joins[index].left_col;
    int32_t s_rel = this->joins[index].right_rel;
    int32_t s_col = this->joins[index].right_col;
    int32_t actual_r = this->rel_names[r_rel];
    int32_t actual_s = this->rel_names[s_rel];

    if (joined[r_rel].getSize() == 0 || joined[s_rel].getSize() == 0 ||
        rel_stats[r_rel][r_col].f == 0 || rel_stats[s_rel][s_col].f == 0 ||
        rel_stats[r_rel][r_col].d == 0 || rel_stats[s_rel][s_col].d == 0) {
      return;
    }

    // Filter σ_A=B (e.g 0 1 | 0.1=0.2&...)
    if (r_rel == s_rel) {
      uint64_t prev_f = rel_stats[r_rel][r_col].f;
      uint64_t prev_d = rel_stats[r_rel][r_col].d;
      if (rel_stats[r_rel][r_col].l > rel_stats[s_rel][s_col].l)
        rel_stats[s_rel][s_col].l = rel_stats[r_rel][r_col].l;
      else
        rel_stats[r_rel][r_col].l = rel_stats[s_rel][s_col].l;

      if (rel_stats[r_rel][r_col].u < rel_stats[s_rel][s_col].u)
        rel_stats[s_rel][s_col].u = rel_stats[r_rel][r_col].u;
      else
        rel_stats[r_rel][r_col].u = rel_stats[s_rel][s_col].u;

      rel_stats[r_rel][r_col].f = rel_stats[s_rel][s_col].f =
          prev_f / (rel_stats[r_rel][r_col].u - rel_stats[r_rel][r_col].l + 1);

      double base = (1 - ((double)rel_stats[r_rel][r_col].f / prev_f));
      double power = ((double)prev_f / prev_d);
      double res = pow(base, power);
      res = prev_d * (1 - res);
      rel_stats[r_rel][r_col].d = rel_stats[s_rel][s_col].d = (uint64_t)res;

      for (size_t i = 0; i < rel_mmap[actual_r].cols; i++)
        if ((int32_t)i != r_col && (int32_t)i != s_col) {
          double base = (1 - ((double)rel_stats[r_rel][r_col].f / prev_f));
          double power =
              ((double)rel_stats[r_rel][i].f / rel_stats[r_rel][i].d);
          double res = pow(base, power);
          res = rel_stats[r_rel][i].d * (1 - res);
          rel_stats[r_rel][i].d = (uint64_t)res;

          rel_stats[r_rel][i].f = rel_stats[r_rel][r_col].f;
        }
    }

    // Self-Join (e.g 0 0 | 0.1=1.1...)
    else if ((actual_r == actual_s) && (r_col == s_col)) {
      uint64_t prev_f = rel_stats[r_rel][r_col].f;
      rel_stats[r_rel][r_col].f =
          (prev_f * prev_f) /
          (rel_stats[r_rel][r_col].u - rel_stats[r_rel][r_col].l + 1);

      for (size_t i = 0; i < rel_mmap[actual_r].cols; i++)
        if ((int32_t)i != r_col)
          rel_stats[r_rel][i].f = rel_stats[r_rel][r_col].f;
    }

    // Join between 2 different relations
    else {
      uint64_t lower = rel_stats[r_rel][r_col].l;
      uint64_t upper = rel_stats[r_rel][r_col].u;
      uint64_t prev_d_r = rel_stats[r_rel][r_col].d;
      uint64_t prev_d_s = rel_stats[s_rel][s_col].d;

      if (rel_stats[s_rel][s_col].l > lower) lower = rel_stats[s_rel][s_col].l;
      if (rel_stats[s_rel][s_col].u < upper) upper = rel_stats[s_rel][s_col].u;

      rel_stats[r_rel][r_col].l = rel_stats[s_rel][s_col].l = lower;
      rel_stats[r_rel][r_col].u = rel_stats[s_rel][s_col].u = upper;

      rel_stats[r_rel][r_col].f = rel_stats[s_rel][s_col].f =
          (rel_stats[r_rel][r_col].f * rel_stats[s_rel][s_col].f) /
          (upper - lower + 1);

      rel_stats[r_rel][r_col].d = rel_stats[s_rel][s_col].d =
          (rel_stats[r_rel][r_col].d * rel_stats[s_rel][s_col].d) /
          (upper - lower + 1);

      for (size_t i = 0; i < rel_mmap[actual_r].cols; i++)
        if ((int32_t)i != r_col) {
          uint64_t prev_f_c = rel_stats[r_rel][i].f;
          uint64_t prev_d_c = rel_stats[r_rel][i].d;
          rel_stats[r_rel][i].f = rel_stats[r_rel][r_col].f;

          double base = (1 - ((double)rel_stats[r_rel][r_col].d / prev_d_r));
          double power = ((double)prev_f_c / prev_d_c);
          double res = pow(base, power);
          res = rel_stats[r_rel][i].d * (1 - res);
          rel_stats[r_rel][i].d = (uint64_t)res;
        }

      for (size_t i = 0; i < rel_mmap[actual_s].cols; i++)
        if ((int32_t)i != s_col) {
          uint64_t prev_f_c = rel_stats[s_rel][i].f;
          uint64_t prev_d_c = rel_stats[s_rel][i].d;
          rel_stats[s_rel][i].f = rel_stats[r_rel][r_col].f;

          double base = (1 - ((double)rel_stats[s_rel][s_col].d / prev_d_s));
          double power = ((double)prev_f_c / prev_d_c);
          double res = pow(base, power);
          res = rel_stats[s_rel][i].d * (1 - res);
          rel_stats[s_rel][i].d = (uint64_t)res;
        }
    }
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::do_query() {
  const size_t filter_count = this->filters.getSize();
  const size_t joins_count = this->joins.getSize();

  for (int32_t i = 0; i < 4; i++) {
    rel_is_filtered[i] = false;
    rel_is_joined[i] = false;
  }

  // Check whether there are filters in order to execute them first
  for (size_t i = 0; i < filter_count; i++) {
    filter_exec(i);
#ifdef Q_OPT
    update_stats(i, 0);
#endif
  }

  // Rearrangement of joins
  // Ascending order of cost
#ifdef Q_OPT
  join_enumeration();
#endif

  for (size_t i = 0; i < joins_count; i++) {
    // swap relations so that the left is always the one that's joined
    if (rel_is_joined[joins[i].left_rel] == false) {
      int32_t temp = joins[i].left_rel;
      int32_t temp_col = joins[i].left_col;
      joins[i].left_rel = joins[i].right_rel;
      joins[i].left_col = joins[i].right_col;
      joins[i].right_rel = temp;
      joins[i].right_col = temp_col;
    }
    do_join(i);
  }

#ifdef Q_OPT
  for (size_t i = 0; i < rel_names.getSize(); i++) {
    delete[] rel_stats[i];
  }
#endif
}

//-----------------------------------------------------------------------------------------

void QueryExec::filter_exec(size_t index) {
  int32_t rel = this->filters[index].rel;
  int32_t actual_rel = this->rel_names[rel];

  int32_t col = this->filters[index].col;
  int32_t lit = this->filters[index].literal;
  operators operation_type = this->filters[index].op;

  simple_vector<int32_t> new_filtered;

  // Relation hasn't been used in a filter predicate before
  if (rel_is_filtered[rel] == false) {
    // Mark the relation as filtered
    rel_is_filtered[rel] = true;

    for (uint64_t row = 0; row < rel_mmap[actual_rel].rows; row++) {
      switch (operation_type) {
        case operators::EQ:
          if ((int32_t)rel_mmap[actual_rel].colptr[col][row] == lit)
            new_filtered.add_back(row);
          break;
        case operators::GREATER:
          if ((int32_t)rel_mmap[actual_rel].colptr[col][row] > lit)
            new_filtered.add_back(row);
          break;
        case operators::LESS:
          if ((int32_t)rel_mmap[actual_rel].colptr[col][row] < lit)
            new_filtered.add_back(row);
          break;
        default:
          std::perror("Unknown operator\n");
          exit(EXIT_FAILURE);
      }
    }
  } else {
    for (size_t i = 0; i < filtered[rel].getSize(); i++) {
      int32_t curr_row = filtered[rel][i];
      switch (operation_type) {
        case operators::EQ:
          if ((int32_t)rel_mmap[actual_rel].colptr[col][curr_row] == lit)
            new_filtered.add_back(curr_row);
          break;
        case operators::GREATER:
          if ((int32_t)rel_mmap[actual_rel].colptr[col][curr_row] > lit)
            new_filtered.add_back(curr_row);
          break;
        case operators::LESS:
          if ((int32_t)rel_mmap[actual_rel].colptr[col][curr_row] < lit)
            new_filtered.add_back(curr_row);
          break;
        default:
          std::perror("Unknown operator\n");
          exit(EXIT_FAILURE);
      }
    }
  }

  filtered[rel].steal(new_filtered);
}

//-----------------------------------------------------------------------------------------

void QueryExec::do_join(size_t join_index) {
  int32_t rel_r = joins[join_index].left_rel;
  int32_t col_r = joins[join_index].left_col;
  int32_t rel_s = joins[join_index].right_rel;
  int32_t col_s = joins[join_index].right_col;
  int32_t actual_rel_r = this->rel_names[rel_r];
  int32_t actual_rel_s = this->rel_names[rel_s];

  memory_map mmap_r = rel_mmap[actual_rel_r];
  memory_map mmap_s = rel_mmap[actual_rel_s];

  simple_vector<int32_t> new_joined[] = {
      simple_vector<int32_t>{}, simple_vector<int32_t>{},
      simple_vector<int32_t>{}, simple_vector<int32_t>{}};

  size_t relr_size;
  size_t rels_size;

  tuple *rtuples = nullptr, *stuples = nullptr;

  // first time both relations are used in a join
  // use filtered rowids if they exist and create new Join array
  if (rel_is_joined[rel_r] == false && rel_is_joined[rel_s] == false) {
    rel_is_joined[rel_r] = true;
    rel_is_joined[rel_s] = true;

    if (rel_is_filtered[rel_r]) {
      relr_size = filtered[rel_r].getSize();
      rtuples = new tuple[relr_size];
      for (size_t i = 0; i < relr_size; i++) {
        int32_t row_id = filtered[rel_r][i];
        // value, rowid
        rtuples[i] = {(int32_t)mmap_r.colptr[col_r][row_id], (int32_t)row_id};
      }
    } else {
      relr_size = mmap_r.rows;
      rtuples = new tuple[relr_size];
      for (size_t i = 0; i < relr_size; i++) {
        rtuples[i] = {(int32_t)mmap_r.colptr[col_r][i], (int32_t)i};
      }
    }

    if (rel_is_filtered[rel_s]) {
      rels_size = filtered[rel_s].getSize();
      stuples = new tuple[rels_size];
      for (size_t i = 0; i < rels_size; i++) {
        int32_t row_id = filtered[rel_s][i];
        stuples[i] = {(int32_t)mmap_s.colptr[col_s][row_id], (int32_t)row_id};
      }
    } else {
      rels_size = mmap_s.rows;
      stuples = new tuple[rels_size];
      for (size_t i = 0; i < rels_size; i++) {
        stuples[i] = {(int32_t)mmap_s.colptr[col_s][i], (int32_t)i};
      }
    }

    relation r(rtuples, relr_size);
    relation s(stuples, rels_size);

    result_mt res = PartitionedHashJoin(r, s);
    for (size_t subresult = 0; subresult < (size_t)res.subresult_count;
         subresult++) {
      result& r = res.r[subresult];
      for (size_t i = 0; i < r.getSize(); i++) {
        new_joined[rel_r].add_back(r[i].rowid_1);
        new_joined[rel_s].add_back(r[i].rowid_2);
      }
    }

    delete[] res.r;

  } else {
    // if r is in joined int32_tmds and s isn't
    // join using existing join result
    // the left rel is always the one thats joined if only 1/2 is joined
    if (rel_is_joined[rel_r] && rel_is_joined[rel_s] == false) {
      rel_is_joined[rel_s] = true;

      relr_size = joined[rel_r].getSize();
      rtuples = new tuple[relr_size];
      for (size_t i = 0; i < relr_size; i++) {
        int32_t row_id = joined[rel_r][i];
        rtuples[i] = {(int32_t)mmap_r.colptr[col_r][row_id], (int32_t)i};
      }

      // create tuples for s
      if (rel_is_filtered[rel_s]) {
        rels_size = filtered[rel_s].getSize();
        stuples = new tuple[rels_size];
        for (size_t i = 0; i < rels_size; i++) {
          int32_t row_id = filtered[rel_s][i];
          // value, rowid
          stuples[i] = {(int32_t)mmap_s.colptr[col_s][row_id], (int32_t)row_id};
        }
      } else {
        rels_size = mmap_s.rows;
        stuples = new tuple[rels_size];
        for (size_t i = 0; i < rels_size; i++) {
          stuples[i] = {(int32_t)mmap_s.colptr[col_s][i], (int32_t)i};
        }
      }

      relation r(rtuples, relr_size);
      relation s(stuples, rels_size);

      result_mt res = PartitionedHashJoin(r, s);

      for (size_t subresult = 0; subresult < (size_t)res.subresult_count;
           subresult++) {
        result& r = res.r[subresult];
        for (size_t i = 0; i < r.getSize(); i++) {
          new_joined[rel_s].add_back(r[i].rowid_2);
          for (size_t j = 0; j < rel_names.getSize(); j++) {
            if ((int32_t)j != rel_s && joined[j].getSize() > 0) {
              new_joined[j].add_back(joined[j][r[i].rowid_1]);
            }
          }
        }
      }

      delete[] res.r;

    } else if (rel_is_joined[rel_r] && rel_is_joined[rel_s]) {
      for (size_t i = 0; i < joined[rel_r].getSize(); i++) {
        int32_t row_id_r = joined[rel_r][i];
        int32_t row_id_s = joined[rel_s][i];
        if (mmap_r.colptr[col_r][row_id_r] == mmap_s.colptr[col_s][row_id_s]) {
          new_joined[rel_r].add_back(row_id_r);
          new_joined[rel_s].add_back(row_id_s);

          for (size_t k = 0; k < rel_names.getSize(); k++) {
            if ((int32_t)k != rel_s && (int32_t)k != rel_r &&
                joined[k].getSize() > 0) {
              new_joined[k].add_back(joined[k][i]);
            }
          }
        }
      }
    }
  }

  for (size_t i = 0; i < rel_names.getSize(); i++) {
    joined[i].steal(new_joined[i]);
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::checksum() {
  int32_t curr_rel;
  int32_t curr_col;
  int32_t curr_row;
  uint64_t sum;
  int32_t actual_rel;

  QueryResults& qr = qres[qindex];

  for (size_t i = 0; i < this->projections.getSize(); i++) {
    curr_rel = this->projections[i].rel;
    curr_col = this->projections[i].col;
    actual_rel = this->rel_names[curr_rel];
    sum = 0;

    if (rel_is_joined[curr_rel]) {
      for (size_t j = 0; j < joined[curr_rel].getSize(); j++) {
        curr_row = joined[curr_rel][j];

        sum += rel_mmap[actual_rel].colptr[curr_col][curr_row];
      }
    } else if (rel_is_filtered[curr_rel]) {
      for (size_t j = 0; j < filtered[curr_rel].getSize(); j++) {
        curr_row = filtered[curr_rel][j];

        sum += rel_mmap[actual_rel].colptr[curr_col][curr_row];
      }
    } else {
      for (size_t j = 0; j < rel_mmap[actual_rel].rows; j++)
        sum += rel_mmap[actual_rel].colptr[curr_col][j];
    }

    qr.sums[qr.projections++] = sum;
  }
}