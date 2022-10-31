#include "acutest.h"
#include "dataForm.h"
#include "hashTable.h"
#include "histogram.h"
#include "list.h"

#include "partitioner.h"

void test_partitioning_function() {
  tuple a{3, 6};
  tuple b{5, 7};
  tuple c{16, 16};
  TEST_CHECK(Partitioner::hash1(a.getKey(), 2) == 3);
  TEST_CHECK(Partitioner::hash1(b.getKey(), 1) == 1);
  TEST_CHECK(Partitioner::hash1(c.getKey(), 4) == 0);
}

void test_partitions_1() {
  tuple a{3, 6};    // 11
  tuple b{5, 7};    // 101
  tuple c{10, 16};  // 1010
  tuple d{12, 0};   // 1100
  tuple e{1, 2};    // 01
  tuple f{11, 10};  // 1011

  tuple* tuples = new tuple[6]{a, b, c, d, e, f};

  relation r(tuples, 6);

  Partitioner p;

  // try with tiny cache size to force partitioning
  relation r_ = p.partition(r, 1);

  TEST_CHECK(r_.getAmount() == r.getAmount());
  TEST_CHECK(r_[0].getKey() == 12);
  TEST_CHECK(r_[1].getKey() == 5);
  TEST_CHECK(r_[2].getKey() == 1);
  TEST_CHECK(r_[3].getKey() == 10);
  TEST_CHECK(r_[4].getKey() == 3);
  TEST_CHECK(r_[5].getKey() == 11);

  // r_.print();
}

void test_partitions_2() {
  tuple a{19, 0};   // 10011
  tuple b{756, 4};  // 1011110100
  tuple c{31, 8};   // 11111
  tuple d{0, 16};   // 0
  tuple* tuples = new tuple[4]{a, b, c, d};
  relation r(tuples, 4);

  Partitioner p;
  relation r_ = p.partition(r, 2);

  TEST_CHECK(r_.getAmount() == r.getAmount());
  TEST_CHECK(r_[0].getKey() == 0);
  TEST_CHECK(r_[1].getKey() == 19);
  TEST_CHECK(r_[2].getKey() == 756);
  TEST_CHECK(r_[3].getKey() == 31);
}

TEST_LIST = {{"test_partinioning_fn", test_partitioning_function},
             {"test_part_pass1", test_partitions_1},
             {"test_part_pass2", test_partitions_2},
             {NULL, NULL}};