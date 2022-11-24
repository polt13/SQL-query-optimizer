#include <cstdio>
#include "map_info.h"
#include "queryParser.h"
#include <cstring>
#include "simple_vector.h"

void QueryParser::parse_query(char* query) {
  char* buffr;

  char* used_relations = strtok_r(query, "|", &buffr);
  simple_vector<char*> names = parse_names(used_relations);

  char* predicates = strtok_r(nullptr, "|", &buffr);
  simple_vector<operations> operations = parse_predicates(predicates);

  // buffr now points to the last part of the query
  char* selections = buffr;
  simple_vector<char*> rel_columns = parse_selections(selections);
}

simple_vector<char*> QueryParser::parse_names(char* rel_string) {
  simple_vector<char*> rel_names;
  char* buffr;
  char* rel = strtok_r(rel_string, " ", &buffr);
  rel_names.add_back(rel);
  while ((rel = strtok_r(nullptr, " ", &buffr))) rel_names.add_back(rel);

  return rel_names;
}

simple_vector<operations> QueryParser::parse_predicates(char* predicates) {
  char* buffr;
  char* buffr2;
  char* predicate = strtok_r(predicates, "&", &buffr);

  simple_vector<operations> result;
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
  result.add_back(operation);

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
    result.add_back(operation);
  }

  return result;
}

simple_vector<char*> QueryParser::parse_selections(char* selections) {
  char* buffr;

  simple_vector<char*> result;

  char* selection = strtok_r(selections, " ", &buffr);

  result.add_back(selection);

  while ((selection = strtok_r(nullptr, " ", &buffr)))
    result.add_back(selection);

  return result;
}