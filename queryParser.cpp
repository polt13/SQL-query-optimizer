#include "queryParser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "map_info.h"
#include "simple_vector.h"

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
  do_query();
  // checksum();

  /* Clear all simple_vectors to prepare for next Query */
  this->rel_names.clear();
  this->predicates.clear();
  this->projections.clear();
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
  char* buffr;
  char* rel = strtok_r(rel_string, " ", &buffr);
  this->rel_names.add_back(rel);
  while ((rel = strtok_r(nullptr, " ", &buffr))) this->rel_names.add_back(rel);
}

void QueryExec::parse_predicates(char* predicates) {
  char* buffr;
  char* buffr2;
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
  char* right = strtok_r(nullptr, op_val, &buffr2);

  operations operation(left, operation_type, right);
  this->predicates.add_back(operation);

  while ((predicate = strtok_r(nullptr, "&", &buffr))) {
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

    left = strtok_r(predicate, op_val, &buffr2);
    right = strtok_r(nullptr, op_val, &buffr2);

    operations operation(left, operation_type, right);
    this->predicates.add_back(operation);
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

void QueryExec::do_query() {
  simple_vector<int64_t> intmd_results[this->rel_names.getSize()];
  /* Check whether there are filters in order to execute them first */
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
}

//-----------------------------------------------------------------------------------------