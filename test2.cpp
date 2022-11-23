#include <cstdio>
#include "dataForm.h"
#include "map_info.h"
#include "queryParser.h"
#include "simple_vector.h"

simple_vector<double> foo() {
  simple_vector<double> x;
  x.add_back(1.25);
  x.add_back(9);
  x.add_back(10.300);
  x.add_back(5);

  return x;
};

int main() {
  QueryParser qp;
  char query[] = "3  |3.2=0.0&3.1=1.0&3.2>3499|1.2 0.1";
  qp.parse_query(query);
}