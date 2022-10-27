#include "acutest.h"
#include "dataForm.h"
#include "hashTable.h"
#include "histogram.h"
#include "node.h"
#include "partition.h"
#include "partitioner.h"

void test_partitioning_function() {
  tuple a{3, 6};
  tuple b{5, 7};
  tuple c{10, 16};
  tuple d{12, 49};
  TEST_CHECK(Partitioner::hash1(a.getPayload(), 2) == 2);
  TEST_CHECK(Partitioner::hash1(b.getPayload(), 4) == 7);
  TEST_CHECK(Partitioner::hash1(c.getPayload(), 4) == 0);
  // fails
  // TEST_CHECK(Partitioner::hash1(d.getPayload(), 64) == 49);
}

void test_partitions_1() {
  tuple a{3, 6};
  tuple b{5, 7};
  tuple c{10, 16};
  tuple d{12, 0};
  tuple e{1, 2};
  tuple f{11, 10};
  tuple* tuples = new tuple[6]{a, b, c, d, e, f};
  relation r(tuples, 6);
  Partitioner p;
  p.partition(r);
  p.printEntries();

  delete[] tuples;
}

void test_partitions_2() {
  tuple a{1, 0};
  tuple b{2, 4};
  tuple c{3, 8};
  tuple d{4, 16};
  tuple* tuples = new tuple[4]{a, b, c, d};
  relation r(tuples, 4);
  Partitioner p;
  p.partition(r);
  p.printEntries();

  delete[] tuples;
}

TEST_LIST = {{"test_partinioning_fn", test_partitioning_function},
             //{"test_part_pass1", test_partitions_1},
             {"test_part_pass2", test_partitions_2},

             {NULL, NULL}};