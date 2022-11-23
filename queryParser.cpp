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
  simple_vector<operations> filters = parse_predicates(predicates);
  // buffr now points to the last part of the query
  char* selections = buffr;
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
  char* first_predicate = strtok_r(predicates, "&", &buffr);
  // split based on the operator
  char operator[2];
  operators operation_type;
  // determine predicate type
  if (std::strchr(first_predicate, '=')) {
    operation_type = operators::EQ;
    std::strcpy(operator, "=");
  } else if (std::strchr(first_predicate, '>')) {
    operation_type = operators::GREATER;
    std::strcpy(operator, ">");
  } else {
    operation_type = operators::LESS;
    std::strcpy(operator, "<");
  }

  return simple_vector<operations>();
}