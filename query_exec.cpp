#include "query_exec.h"

#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dataForm.h"
#include "map_info.h"
#include "partitioner.h"
#include "simple_vector.h"

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
  do_query();
  checksum();

  // Clear all simple_vectors to prepare for next Query
  clear();
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

void QueryExec::parse_names(char* rel_string) {
  char *buffr, *ignore;
  char* rel;

  while ((rel = strtok_r(rel_string, " ", &buffr))) {
    this->rel_names.add_back(std::strtol(rel, &ignore, 10));
    rel_string = nullptr;
  }
}

void QueryExec::parse_predicates(char* predicates) {
  char* ignore;
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
      long literal = std::strtol(left, &ignore, 10);

      if (operation_type == operators::GREATER)
        operation_type = operators::LESS;
      else if (operation_type == operators::LESS)
        operation_type = operators::GREATER;
      else
        operation_type = operators::EQ;

      char* right = buffr2;

      long right_rel = std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);

      long right_col = std::strtol(buffr3, &ignore, 10);

      filter myfilter(right_rel, right_col, operation_type, literal);
      this->filters.add_back(myfilter);

    } else {
      // e.g 0.2 (relation.column) - Could be filter OR join
      long left_rel = std::strtol(strtok_r(left, ".", &buffr3), &ignore, 10);
      long left_col = std::strtol(buffr3, &ignore, 10);

      char* right = buffr2;
      if (std::strchr(right, '.') == nullptr) {
        // right contains literal - 100% filter
        long literal = std::strtol(right, &ignore, 10);

        filter myfilter(left_rel, left_col, operation_type, literal);
        this->filters.add_back(myfilter);
      } else {
        // e.g 1.4 (relation.column) - 100% join (but can be same relation!!!)
        long right_rel =
            std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);
        long right_col = std::strtol(buffr3, &ignore, 10);

        join myjoin(left_rel, left_col, operation_type, right_rel, right_col);
        this->joins.add_back(myjoin);
      }
    }
    predicates = nullptr;
  }
}

void QueryExec::parse_selections(char* selections) {
  char *buffr, *buffr2;
  char* ignore;
  char* selection;

  while ((selection = strtok_r(selections, " ", &buffr))) {
    char* rel = strtok_r(selection, ".", &buffr2);
    char* col = buffr2;

    this->projections.add_back(project_rel{std::strtol(rel, &ignore, 10),
                                           std::strtol(col, &ignore, 10)});

    selections = nullptr;
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::do_query() {
  const size_t filter_count = this->filters.getSize();
  const size_t joins_count = this->joins.getSize();

  for (size_t i = 0; i < 4; i++) {
    rel_is_filtered[i] = false;
    rel_is_joined[i] = false;
  }

  // Check whether there are filters in order to execute them first
  for (size_t i = 0; i < filter_count; i++) filter_exec(i);

  for (size_t i = 0; i < joins_count; i++) {
    // swap relations so that the left is always the one that's joined
    if (rel_is_joined[joins[i].left_rel] == false) {
      int64_t temp = joins[i].left_rel;
      int64_t temp_col = joins[i].left_col;
      joins[i].left_rel = joins[i].right_rel;
      joins[i].left_col = joins[i].right_col;
      joins[i].right_rel = temp;
      joins[i].right_col = temp_col;
    }
    do_join(i);
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::filter_exec(size_t index) {
  int64_t rel = this->filters[index].rel;
  int64_t actual_rel = this->rel_names[rel];

  int64_t col = this->filters[index].col;
  int64_t lit = this->filters[index].literal;
  operators operation_type = this->filters[index].op;

  simple_vector<int64_t> new_filtered;

  // Relation hasn't been used in a filter predicate before
  if (rel_is_filtered[rel] == false) {
    // Mark the relation as filtered
    rel_is_filtered[rel] = true;

    for (uint64_t row = 0; row < rel_mmap[actual_rel].rows; row++) {
      switch (operation_type) {
        case operators::EQ:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][row] == lit)
            new_filtered.add_back(row);
          break;
        case operators::GREATER:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][row] > lit)
            new_filtered.add_back(row);
          break;
        case operators::LESS:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][row] < lit)
            new_filtered.add_back(row);
          break;
        default:
          std::perror("Unknown operator\n");
          exit(EXIT_FAILURE);
      }
    }
  } else {
    for (size_t i = 0; i < filtered[rel].getSize(); i++) {
      int64_t curr_row = filtered[rel][i];
      switch (operation_type) {
        case operators::EQ:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][curr_row] == lit)
            new_filtered.add_back(curr_row);
          break;
        case operators::GREATER:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][curr_row] > lit)
            new_filtered.add_back(curr_row);
          break;
        case operators::LESS:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][curr_row] < lit)
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

void QueryExec::do_join(size_t join_index) {
  int64_t rel_r = joins[join_index].left_rel;
  int64_t col_r = joins[join_index].left_col;
  int64_t rel_s = joins[join_index].right_rel;
  int64_t col_s = joins[join_index].right_col;
  int64_t actual_rel_r = this->rel_names[rel_r];
  int64_t actual_rel_s = this->rel_names[rel_s];

  memory_map mmap_r = rel_mmap[actual_rel_r];
  memory_map mmap_s = rel_mmap[actual_rel_s];

  simple_vector<int64_t> new_joined[] = {
      simple_vector<int64_t>{100}, simple_vector<int64_t>{100},
      simple_vector<int64_t>{100}, simple_vector<int64_t>{100}};

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
        int64_t row_id = filtered[rel_r][i];
        // value, rowid
        rtuples[i] = {(int64_t)mmap_r.colptr[col_r][row_id], (int64_t)row_id};
      }
    } else {
      relr_size = mmap_r.rows;
      rtuples = new tuple[relr_size];
      for (size_t i = 0; i < relr_size; i++) {
        rtuples[i] = {(int64_t)mmap_r.colptr[col_r][i], (int64_t)i};
      }
    }

    if (rel_is_filtered[rel_s]) {
      rels_size = filtered[rel_s].getSize();
      stuples = new tuple[rels_size];
      for (size_t i = 0; i < rels_size; i++) {
        int64_t row_id = filtered[rel_s][i];
        stuples[i] = {(int64_t)mmap_s.colptr[col_s][row_id], (int64_t)row_id};
      }
    } else {
      rels_size = mmap_s.rows;
      stuples = new tuple[rels_size];
      for (size_t i = 0; i < rels_size; i++) {
        stuples[i] = {(int64_t)mmap_s.colptr[col_s][i], (int64_t)i};
      }
    }

    relation r(rtuples, relr_size);
    relation s(stuples, rels_size);
    result res = PartitionedHashJoin(r, s);

    for (size_t i = 0; i < res.getSize(); i++) {
      new_joined[rel_r].add_back(res[i].rowid_1);
      new_joined[rel_s].add_back(res[i].rowid_2);
    }
    // In case of 2 intermediate results
    for (size_t i = 0; i < this->rel_names.getSize(); i++) {
      if ((int64_t)i != rel_r && (int64_t)i != rel_s &&
          (int64_t)joined[i].getSize() > 0)
        new_joined[i].steal(joined[i]);
    }
  } else {
    // if r is in joined intmds and s isn't
    // join using existing join result
    // the left rel is always the one thats joined if only 1/2 is joined
    if (rel_is_joined[rel_r] && rel_is_joined[rel_s] == false) {
      rel_is_joined[rel_s] = true;

      relr_size = joined[rel_r].getSize();
      rtuples = new tuple[relr_size];
      for (size_t i = 0; i < relr_size; i++) {
        int64_t row_id = joined[rel_r][i];
        rtuples[i] = {(int64_t)mmap_r.colptr[col_r][row_id], (int64_t)i};
      }

      // create tuples for s
      if (rel_is_filtered[rel_s]) {
        rels_size = filtered[rel_s].getSize();
        stuples = new tuple[rels_size];
        for (size_t i = 0; i < rels_size; i++) {
          int64_t row_id = filtered[rel_s][i];
          // value, rowid
          stuples[i] = {(int64_t)mmap_s.colptr[col_s][row_id], (int64_t)row_id};
        }
      } else {
        rels_size = mmap_s.rows;
        stuples = new tuple[rels_size];
        for (size_t i = 0; i < rels_size; i++) {
          stuples[i] = {(int64_t)mmap_s.colptr[col_s][i], (int64_t)i};
        }
      }

      relation r(rtuples, relr_size);
      relation s(stuples, rels_size);
      result res = PartitionedHashJoin(r, s);

      for (size_t i = 0; i < res.getSize(); i++) {
        new_joined[rel_s].add_back(res[i].rowid_2);
        for (size_t j = 0; j < rel_names.getSize(); j++) {
          if ((int64_t)j != rel_s && joined[j].getSize() > 0) {
            new_joined[j].add_back(joined[j][res[i].rowid_1]);
          }
        }
      }
    } else if (rel_is_joined[rel_r] && rel_is_joined[rel_s]) {
      for (size_t i = 0; i < joined[rel_r].getSize(); i++) {
        int64_t row_id_r = joined[rel_r][i];
        int64_t row_id_s = joined[rel_s][i];
        if (mmap_r.colptr[col_r][row_id_r] == mmap_s.colptr[col_s][row_id_s]) {
          new_joined[rel_r].add_back(row_id_r);
          new_joined[rel_s].add_back(row_id_s);

          for (size_t k = 0; k < rel_names.getSize(); k++) {
            if ((int64_t)k != rel_s && (int64_t)k != rel_r &&
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

void QueryExec::checksum() {
  int64_t curr_rel;
  int64_t curr_col;
  int64_t curr_row;
  int64_t sum;
  int64_t actual_rel;

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

    if (sum == 0)
      std::printf("NULL");
    else {
      std::printf("%ld", sum);
    }
    if (i < this->projections.getSize() - 1) {
      std::printf(" ");
    }
  }
  std::printf("\n");
}

//-----------------------------------------------------------------------------------------

void QueryExec::clear() {
  this->rel_names.clear();
  this->joins.clear();
  this->filters.clear();
  this->projections.clear();
  for (int64_t i = 0; i < 4; i++) {
    joined[i].clear();
    filtered[i].clear();
  }
}