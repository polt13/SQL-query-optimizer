#include "query_exec.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "map_info.h"
#include "simple_vector.h"

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
  do_query();
  //  checksum();

  /* Clear all simple_vectors to prepare for next Query */
  clear();
}

//-----------------------------------------------------------------------------------------

void QueryExec::parse_query(char* query) {
  char* buffr;

  char* used_relations = strtok_r(query, "|", &buffr);
  parse_names(used_relations);

  // for (size_t i = 0; i < rel_names.getSize(); i++) {
  //   std::printf("rel %ld\n", rel_names[i]);
  // }

  char* predicates = strtok_r(nullptr, "|", &buffr);
  parse_predicates(predicates);

  // for (size_t i = 0; i < joins.getSize(); i++) {
  //   std::fprintf(stderr, "%ld.%ld %c %ld.%ld\n", joins[i].left_rel,
  //                joins[i].left_col,
  //                (joins[i].op == 0) ? ('=') : ((joins[i].op == 1) ? '>' :
  //                '<'), joins[i].right_rel, joins[i].right_col);
  // }
  // for (size_t i = 0; i < filters.getSize(); i++) {
  //   std::fprintf(
  //       stderr, "%ld.%ld %c %ld\n", filters[i].left_rel, filters[i].left_col,
  //       (filters[i].op == 0) ? ('=') : ((filters[i].op == 1) ? '>' : '<'),
  //       filters[i].literal);
  // }

  // buffr now points to the last part of the query
  char* selections = buffr;
  parse_selections(selections);

  //   for (size_t i = 0; i < projections.getSize(); i++) {
  //     std::printf("projection rel %ld col %ld\n", projections[i].rel,
  //                 projections[i].col);
  //   }
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
  simple_vector<int64_t>* intmd_results =
      new simple_vector<int64_t>[this->rel_names.getSize()];
  // Check whether there are filters in order to execute them first
  for (size_t i = 0; i < this->filters.getSize(); i++) {
    filter_exec(i, intmd_results[filters[i].rel]);
  }

  // Check for Self-joins
  for (size_t i = 0; i < this->joins.getSize(); i++) {
    if (joins[i].left_rel == joins[i].right_rel) {
      // Self-join found - Execute it
    }
  }

  // Execute the rest - Partitioned Hash Joins (PHJs)
  for (size_t i = 0; i < this->joins.getSize(); i++) {
    if (joins[i].left_rel != joins[i].right_rel) {
      // Check if both relations exist in intermediate results
      // if (...) { ... }
    }
  }

  // Execute Partitioned Hash Joins (PHJs)
  for (size_t i = 0; i < this->joins.getSize(); i++) {
    if (joins[i].left_rel != joins[i].right_rel) {
      // Check if ΝΟΤ both relations exist in intermediate results
      // if (...) {
        // PHJ found - Execute it
        // ...
      //}
    }
  }

  // Done with all predicates
  // Checksum on given projections
  // Execute Checksum
  checksum(intmd_results);

  delete[] intmd_results;
}

void QueryExec::filter_exec(size_t index,
                            simple_vector<int64_t>& intmd_result) {
  int64_t rel = this->filters[index].rel;
  int64_t col = this->filters[index].col;
  int64_t lit = this->filters[index].literal;
  operators operation_type = this->filters[index].op;

  // Check if the relation has been through a predicate yet
  if (intmd_result.getSize() == 0) {
    // First time applying a predicate on this relation
    // We have to traverse the initial relation
    for (uint64_t row = 0; row < rel_mmap[rel].rows; row++) {
      switch (operation_type) {
        case operators::EQ:
          if ((int64_t)rel_mmap[rel].colptr[col][row] == lit)
            intmd_result.add_back(row);
        case operators::GREATER:
          if ((int64_t)rel_mmap[rel].colptr[col][row] > lit)
            intmd_result.add_back(row);
        case operators::LESS:
          if ((int64_t)rel_mmap[rel].colptr[col][row] < lit)
            intmd_result.add_back(row);
        default:
          std::perror("Unknown operator\n");
          exit(EXIT_FAILURE);
      }
    }
  } else {
    // Relation has already been through a predicate before
    // We have to traverse the intermediate results of the
    // corresponding relation
    simple_vector<int64_t> new_v;
    for (size_t i = 0; i < intmd_result.getSize(); i++) {
      int64_t curr_row = intmd_result[i];

      switch (operation_type) {
        case operators::EQ:
          if ((int64_t)rel_mmap[rel].colptr[col][curr_row] == lit)
            new_v.add_back(curr_row);
        case operators::GREATER:
          if ((int64_t)rel_mmap[rel].colptr[col][curr_row] > lit)
            new_v.add_back(curr_row);
        case operators::LESS:
          if ((int64_t)rel_mmap[rel].colptr[col][curr_row] < lit)
            new_v.add_back(curr_row);
        default:
          std::perror("Unknown operator\n");
          exit(EXIT_FAILURE);
      }
    }
    // Replace relation's intermediate results with the new simple_vector
    intmd_result = new_v;
  }
}

void QueryExec::checksum(simple_vector<int64_t> intmd_results[]) {
  int64_t curr_rel;
  int64_t curr_col;
  int64_t curr_row;
  int64_t sum;

  for (size_t i = 0; this->projections.getSize(); i++) {
    curr_rel = this->projections[i].rel;
    curr_col = this->projections[i].col;
    sum = 0;

    for (size_t j = 0; j < intmd_results[curr_rel].getSize(); j++) {
      curr_row = intmd_results[curr_rel][j];

      sum += rel_mmap[curr_rel].colptr[curr_col][curr_row];
    }

    std::fprintf(stdin, "%ld ", sum);
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::clear() {
  this->rel_names.clear();
  this->joins.clear();
  this->filters.clear();
  this->projections.clear();
}