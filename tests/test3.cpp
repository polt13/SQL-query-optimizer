#include <cstdio>
#include "dataForm.h"
#include "map_info.h"
#include "query_exec.h"
#include "simple_vector.h"

int main() {
  QueryExec qe;
  char query[] =
      "3 0 1 |3.2=0.0&3.1=1.0&3.2>3499&3.9<1.0&& 1 > 5.2 && 5 = 1.3|1.2 0.1 "
      "5.3";
  qe.execute(query);
}