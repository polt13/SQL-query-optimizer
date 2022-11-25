#include "queryParser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "map_info.h"
#include "simple_vector.h"

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
  // do_query();
  //  checksum();

  for (size_t i = 0; i < joins.getSize(); i++) {
    std::fprintf(stderr, "%ld.%ld %d %ld.%ld\n", joins[i].left_rel,
                 joins[i].left_col, joins[i].op, joins[i].right_rel,
                 joins[i].right_col);
  }
  for (size_t i = 0; i < filters.getSize(); i++) {
    std::fprintf(stderr, "%ld.%ld %d %ld\n", filters[i].left_rel,
                 filters[i].left_col, filters[i].op, filters[i].literal);
  }

  /* Clear all simple_vectors to prepare for next Query */
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
  char* rel = strtok_r(rel_string, " ", &buffr);
  this->rel_names.add_back(std::strtol(rel, &ignore, 10));
  while ((rel = strtok_r(nullptr, " ", &buffr)))
    this->rel_names.add_back(std::strtol(rel, &ignore, 10));
}

void QueryExec::parse_predicates(char* predicates) {
  char* ignore;
  char *buffr, *buffr2, *buffr3;
  char* predicate = strtok_r(predicates, "&", &buffr);

  // split based on the operator
  char op_val[2];

  operators operation_type;
  // determine predicate type
  if (std::strchr(predicate, '=')) {
    operation_type = operators::EQ;
    std::strcpy(op_val, "=");
  } else if (std::strchr(predicate, '>')) {
    operation_type = operators::GREATER;
    std::strcpy(op_val, ">");
  } else {
    operation_type = operators::LESS;
    std::strcpy(op_val, "<");
  }

  char* left = strtok_r(predicate, op_val, &buffr2);
  if (std::strchr(left, '.') == nullptr) {
    // left contains literal - 100% filter
    int64_t literal = std::strtol(left, &ignore, 10);

    if (operation_type == operators::GREATER)
      operation_type = operators::LESS;
    else if (operation_type == operators::LESS)
      operation_type = operators::GREATER;
    else
      operation_type = operators::EQ;

    char* right = buffr2;
    int64_t right_rel = std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);
    int64_t right_col =
        std::strtol(strtok_r(nullptr, ".", &buffr3), &ignore, 10);

    filter myfilter(right_rel, right_col, operation_type, literal);
    this->filters.add_back(myfilter);
  } else {
    // e.g 0.2 (relation.column) - Could be filter OR join
    int64_t left_rel = std::strtol(strtok_r(left, ".", &buffr3), &ignore, 10);
    int64_t left_col =
        std::strtol(strtok_r(nullptr, ".", &buffr3), &ignore, 10);

    char* right = buffr2;
    if (std::strchr(right, '.') == nullptr) {
      // right contains literal - 100% filter
      int64_t literal = std::strtol(right, &ignore, 10);

      filter myfilter(left_rel, left_col, operation_type, literal);
      this->filters.add_back(myfilter);
    } else {
      // e.g 1.4 (relation.column) - 100% join (but can be same relation!!!)
      int64_t right_rel =
          std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);
      int64_t right_col =
          std::strtol(strtok_r(nullptr, ".", &buffr3), &ignore, 10);

      join myjoin(left_rel, left_col, operation_type, right_rel, right_col);
      this->joins.add_back(myjoin);
    }
  }

  while ((predicate = strtok_r(nullptr, "&", &buffr))) {
    // determine predicate type
    if (std::strchr(predicate, '=')) {
      operation_type = operators::EQ;
      std::strcpy(op_val, "=");
    } else if (std::strchr(predicate, '>')) {
      operation_type = operators::GREATER;
      std::strcpy(op_val, ">");
    } else {
      operation_type = operators::LESS;
      std::strcpy(op_val, "<");
    }

    char* left = strtok_r(predicate, op_val, &buffr2);
    if (std::strchr(left, '.') == nullptr) {
      // left contains literal - 100% filter
      int64_t literal = std::strtol(left, &ignore, 10);

      if (operation_type == operators::GREATER)
        operation_type = operators::LESS;
      else if (operation_type == operators::LESS)
        operation_type = operators::GREATER;
      else
        operation_type = operators::EQ;

      char* right = buffr2;
      int64_t right_rel =
          std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);
      int64_t right_col =
          std::strtol(strtok_r(nullptr, ".", &buffr3), &ignore, 10);

      filter myfilter(right_rel, right_col, operation_type, literal);
      this->filters.add_back(myfilter);
    } else {
      // e.g 0.2 (relation.column) - Could be filter OR join
      int64_t left_rel = std::strtol(strtok_r(left, ".", &buffr3), &ignore, 10);
      int64_t left_col =
          std::strtol(strtok_r(nullptr, ".", &buffr3), &ignore, 10);

      char* right = buffr2;
      if (std::strchr(right, '.') == nullptr) {
        // right contains literal - 100% filter
        int64_t literal = std::strtol(right, &ignore, 10);

        filter myfilter(left_rel, left_col, operation_type, literal);
        this->filters.add_back(myfilter);
      } else {
        // e.g 1.4 (relation.column) - 100% join (but can be same relation!!!)
        int64_t right_rel =
            std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);
        int64_t right_col =
            std::strtol(strtok_r(nullptr, ".", &buffr3), &ignore, 10);

        join myjoin(left_rel, left_col, operation_type, right_rel, right_col);
        this->joins.add_back(myjoin);
      }
    }
  }
}

void QueryExec::parse_selections(char* selections) {
  char* buffr;

  char* selection = strtok_r(selections, " ", &buffr);

  this->projections.add_back(selection);

  while ((selection = strtok_r(nullptr, " ", &buffr)))
    this->projections.add_back(selection);
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

void QueryExec::clear() {
  this->rel_names.clear();
  this->joins.clear();
  this->filters.clear();
  this->projections.clear();
}