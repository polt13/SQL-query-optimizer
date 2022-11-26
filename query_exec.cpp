#include "query_exec.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdlib>

#include "map_info.h"
#include "dataForm.h"
#include "simple_vector.h"

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
  // do_query();
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
  for (size_t i = 0; i < projections.getSize(); i++) {
    std::printf("projection rel %ld col %ld\n", projections[i].rel,
                projections[i].col);
  }
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

/* void QueryExec::do_query() {
  simple_vector<int64_t> intmd_results[this->rel_names.getSize()];
  // Check whether there are filters in order to execute them first
  for (size_t i = 0; i < this->predicates.getSize(); i++) {
    operators curr_op = predicates[i].op;
    if (curr_op == operators::GREATER || curr_op == operators::LESS) {
      // execute filter predicate
      //
    } else if (curr_op == operators::EQ) {
      char* curr_left = this->predicates[i].left;

      if (std::strchr(curr_left, '.') == nullptr) {
        // execute filter predicate
        // left is literal
        //
      } else {
        char* curr_right = this->predicates[i].right;
        if (std::strchr(curr_right, '.') == nullptr) {
          // execute filter predicate
          // right is literal
          //
        }
      }
    } else {
      std::perror("Unknown operator\n");
      exit(EXIT_FAILURE);
    }
  }
} */

//-----------------------------------------------------------------------------------------

// rowids = intermediate for relation
simple_vector<result_item> self_join(simple_vector<long int>& rowids,
                                     long int relation, long int col1,
                                     long int col2) {
  simple_vector<result_item> next;

  uint64_t* relation_col1 = rel_mmap[relation].colptr[col1];
  uint64_t* relation_col2 = rel_mmap[relation].colptr[col2];

  const size_t row_count = rowids.getSize();

  for (size_t i = 0; i < row_count; i++) {
    int rowid_left = rowids[i];
    for (size_t j = 0; j < row_count; j++) {
      int rowid_right = rowids[j];
      if (relation_col1[rowid_left] == relation_col2[rowid_right]) {
        next.add_back(result_item{rowid_left, rowid_right});
      }
    }
  }

  return next;
}

void QueryExec::clear() {
  this->rel_names.clear();
  this->joins.clear();
  this->filters.clear();
  this->projections.clear();
}