#include "queryParser.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "map_info.h"
#include "simple_vector.h"

void QueryExec::execute(char* query) {
  parse_query(query);
  do_query();
  // checksum();

  /* Clear all simple_vectors to prepare for next Query */
  this->rel_names.clear();
  this->predicates.clear();
  this->projections.clear();
}

void QueryExec::parse_query(char* query) {
  char* buffr;

  char* used_relations = strtok_r(query, "|", &buffr);
  parse_names(used_relations);
  for (size_t i = 0; i < rel_names.getSize(); i++) {
    std::printf("rel %ld\n", rel_names[i]);
  }

  char* predicates = strtok_r(nullptr, "|", &buffr);
  parse_predicates(predicates);

  // buffr now points to the last part of the query
  char* selections = buffr;
  parse_selections(selections);
  for (size_t i = 0; i < projections.getSize(); i++) {
    std::printf(" rel_idx %ld rel_col %ld\n", projections[i].rel,
                projections[i].col);
  }
}

void QueryExec::parse_names(char* rel_string) {
  char* buffr;
  // ignored but used by strol..
  char* ignore;
  char* rel = strtok_r(rel_string, " ", &buffr);
  this->rel_names.add_back(std::strtol(rel, &ignore, 10));
  while ((rel = strtok_r(nullptr, " ", &buffr)))
    this->rel_names.add_back(std::strtol(rel, &ignore, 10));
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
  char* buffr2;

  char* ignore;
  char* selection = strtok_r(selections, " ", &buffr);

  char* rel = strtok_r(selection, ".", &buffr2);
  char* col = buffr2;

  this->projections.add_back(project_rel{std::strtol(rel, &ignore, 10),
                                         std::strtol(col, &ignore, 10)});

  while ((selection = strtok_r(nullptr, " ", &buffr))) {
    rel = strtok_r(selection, ".", &buffr2);
    col = buffr2;

    this->projections.add_back(project_rel{std::strtol(rel, &ignore, 10),
                                           std::strtol(col, &ignore, 10)});
  }
}

void QueryExec::do_query() {
  /* Check whether there is a filter
   * in order to execute it first
   */
  for (size_t i = 0; i < this->predicates.getSize(); i++) {
    operators curr_op = predicates[i].op;
    if (curr_op == operators::GREATER || curr_op == operators::LESS) {
      // execute filter predicate
    }
  }
}