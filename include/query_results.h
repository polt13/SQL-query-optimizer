#ifndef QRESULTS
#define QRESULTS
#include <cstdint>
struct QueryResults {
  int projections;
  uint64_t sums[4];  // convention -- up to 6
  QueryResults() : projections{} {}
};

#endif