#ifndef DF_H
#define DF_H
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "simple_vector.h"

/* Type definition for a tuple */
class tuple {
  int32_t key;      // value (R.a)
  int32_t payload;  // rowID

 public:
  int32_t getKey() { return key; }
  int32_t getPayload() { return payload; }

  tuple(int32_t key, int32_t payload) : key{key}, payload{payload} {}

  tuple() = default;
};

/*
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
class relation {
  tuple *tuples;
  int32_t num_tuples;

 public:
  relation(tuple *tuples, int32_t num_tuples)
      : tuples{tuples}, num_tuples{num_tuples} {}

  relation(const relation &other) {
    num_tuples = other.num_tuples;
    tuples = new tuple[num_tuples];
    // copy array
    std::memmove(tuples, other.tuples, sizeof(tuple) * num_tuples);
  }

  ~relation() { delete[] tuples; }

  tuple &operator[](int32_t index) { return tuples[index]; }

  int32_t getAmount() { return num_tuples; }
};

struct result_item {
  int32_t rowid_1;
  int32_t rowid_2;
};

struct result {
 private:
  simple_vector<result_item> pairs;
  char pad[40];

 public:
  void push(const result_item &r) { pairs.add_back(r); }

  size_t getSize() const { return pairs.getSize(); }

  result() = default;

  result(const result &other) : pairs{other.pairs} {}

  result_item &operator[](int32_t index) { return pairs[index]; }
};

struct result_mt {
  result *r;
  int32_t subresult_count;  // how many different results have been created
  // avoid having to join them in one
  result_mt(result *r, int32_t subresult_count)
      : r{r}, subresult_count{subresult_count} {}
};

#endif