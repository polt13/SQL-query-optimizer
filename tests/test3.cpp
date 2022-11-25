#include <cstdio>
#include "dataForm.h"
#include "map_info.h"
#include "queryParser.h"
#include "simple_vector.h"

int main() {
  QueryExec qe;
  char query[] = "3 0 1 |3.2=0.0&3.1=1.0&3.2>3499&3.9<1.0|1.2 0.1";
  qe.execute(query);
}