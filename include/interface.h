#include <stdint.h>

//#define n // Least Significant Bits (LSB)
#define N 1
#define M 1

/* Type definition for a tuple */
struct tuple
{
    int32_t key;     // row_id
    int32_t payload; // value
};

class bucket
{
    private:
    tuple *tuples[N];
    // tuple *mytuple;

    public:
};

class hashTable
{
    private:
    bucket *buckets[M];

    public:
};

/*
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
struct relation
{
    tuple *tuples;
    uint32_t num_tuples;
};

/*
 * Type definition for result.
 * It consists of an array of tuples and a size of the relation.
 */
struct result
{
    tuple *tuples;
    uint32_t num_tuples;
};

/* Partitioned Hash Join */
result *PartitionedHashJoin(relation *relR, relation *relS);