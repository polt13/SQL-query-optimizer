#ifndef DF_H
#define DF_H
#include <cstdint>
/* Type definition for a tuple */
class tuple {
  int64_t key;      // row_id
  int64_t payload;  // value

 public:
  int64_t getKey() { return key; }
  int64_t getPayload() { return payload; }
};

/*
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
class relation {
  tuple *tuples;
  uint64_t num_tuples;

 public:
  relation(uint64_t num_tuples) {
    this->num_tuples = num_tuples;
    this->tuples = new tuple[this->num_tuples];
  }

  uint64_t getAmount() { return num_tuples; }
  tuple &getTuple(uint64_t index) { return tuples[index]; }

  ~relation() { delete tuples; }
};

/*
 * Type definition for result.
 * It consists of an array of tuples and a size of the relation.
 */
class result {
  tuple *tuples;
  uint64_t num_tuples;
};

/* Partitioned Hash Join */
result *PartitionedHashJoin(relation *relR, relation *relS);
#endif