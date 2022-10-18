#include <cstdint>

/* Type definition for a tuple */
class tuple {
  int32_t key;      // row_id
  int32_t payload;  // value

 public:
  int32_t getKey() { return key; }
  int32_t getPayload() { return payload; }
};

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

/*
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
class relation {
  tuple *tuples;
  uint32_t num_tuples;

 public:
  relation(uint32_t num_tuples) {
    this->num_tuples = num_tuples;
    this->tuples = new tuple[this->num_tuples];
  }

  uint32_t getAmount() { return num_tuples; }
  tuple &getTuple(uint32_t index) { return tuples[index]; }

  ~relation() { delete tuples; }
};

/*
 * Type definition for result.
 * It consists of an array of tuples and a size of the relation.
 */
class result {
  tuple *tuples;
  uint32_t num_tuples;
};

/* Partitioned Hash Join */
result *PartitionedHashJoin(relation *relR, relation *relS);