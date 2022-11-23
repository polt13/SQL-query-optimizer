#include <cstdio>
#include "map_info.h"
#include "queryParser.h"
#include <cstring>
#include "simple_vector.h"

void QueryParser::parse_query(char* query) {
  char* buffr;
  char* used_relations = strtok_r(query, "|", &buffr);
  simple_vector<char*> names = parse_names(used_relations);
  for (long i = 0; i < names.getSize(); i++) printf("%s\n", names[i]);
  char* predicates = strtok_r(nullptr, "|", &buffr);
  // buffr now points to the last part of the query
  char* selections = buffr;
}

simple_vector<char*> QueryParser::parse_names(char* rel_string) {
  simple_vector<char*> rel_names;
  char* buffr;
  char* rel = strtok_r(rel_string, " ", &buffr);
  rel_names.add_back(rel);
  while (rel = strtok_r(nullptr, " ", &buffr)) rel_names.add_back(rel);

  return rel_names;
}