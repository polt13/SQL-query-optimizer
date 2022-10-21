#ifndef HT_H
#define HT_H
#include "dataForm.h"
class bucket {
 private:
  tuple *tuples;
  // tuple *mytuple;
  uint64_t tuple_count;
};

class hashTable {
 private:
  bucket *buckets;
  uint64_t bucket_count;
};
#endif