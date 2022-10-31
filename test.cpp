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

/* HT's Hash Function
 * ------------------
 */
void test_HThash2() {
  tuple a{3, 6};
  tuple b{5, 7};
  tuple c{10, 16};
  tuple d{125, 0};
  tuple e{1, 2};
  tuple f{1531, 10};

  hashTable h1(0);
  hashTable h2(1);
  hashTable h3(4);
  hashTable h4(8);
  hashTable h5(6);
  hashTable h6(62);

  TEST_CHECK(h1.hash2(a.getKey()) == -1);  // 3 % 0
  TEST_CHECK(h2.hash2(b.getKey()) == 0);   // 5 % 1
  TEST_CHECK(h3.hash2(c.getKey()) == 2);   // 10 % 4
  TEST_CHECK(h4.hash2(d.getKey()) == 5);   // 125 % 8
  TEST_CHECK(h5.hash2(e.getKey()) == 1);   // 1 % 6
  TEST_CHECK(h6.hash2(f.getKey()) == 43);  // 1531 % 62
}

/* Normal Hash Table Insert
 * ------------------------
 * Index "hashVal" is empty
 */
void test_HTinsert1() {
  tuple a{10, 1};  // 10 % 4 = 2
  hashTable h(4);

  h.insert(&a);
  TEST_CHECK(h.getBucket(2)->getTuples().getRoot()->mytuple == &a);
  TEST_CHECK(h.getBucket(2)->getOccupied() == true);
  TEST_CHECK(h.getBucket(2)->getBitmapIndex(0) == true);
}

/* Same Key HT Insert
 * ------------------
 * Two tuples with same Key but different Payload (rowID)
 * Second tuple has to be inserted (append) into first tuple's List
 */
void test_HTinsert2() {
  tuple a{10, 1};  // 10 % 4 = 2
  tuple b{10, 2};  // Same key but different row

  hashTable h(4);

  h.insert(&a);
  h.insert(&b);
  TEST_CHECK(h.getBucket(2)->getTuples().getRoot()->mytuple == &a);
  TEST_CHECK(h.getBucket(2)->getOccupied() == true);
  TEST_CHECK(h.getBucket(2)->getBitmapIndex(0) == true);
  TEST_CHECK(h.getBucket(2)->getTuples().find(b) == true);
}

/* Empty NBHD Slot HT Insert
 * -------------------------
 * Index "hashVal" already Occupied, find empty slot
 * Empty slot already in NBHD_Size range, no need to swap
 * with anything. Just insert in index j instead.
 * Includes the case when exceeding HT's Size (start from 0)
 */
void test_HTinsert3() {
  tuple a{10, 1};  // 10 % 4 = 2
  tuple b{14, 3};  // 14 % 4 = 2
  tuple c{18, 6};  // 18 % 4 = 2

  hashTable h(4);

  h.insert(&a);
  h.insert(&b);
  h.insert(&c);
  TEST_CHECK(h.getBucket(2)->getTuples().getRoot()->mytuple == &a);
  TEST_CHECK(h.getBucket(2)->getOccupied() == true);
  TEST_CHECK(h.getBucket(2)->getBitmapIndex(0) == true);
  TEST_CHECK(h.getBucket(2)->getBitmapIndex(1) == true);
  TEST_CHECK(h.getBucket(2)->getBitmapIndex(2) == true);

  TEST_CHECK(h.getBucket(3)->getTuples().getRoot()->mytuple == &b);
  TEST_CHECK(h.getBucket(3)->getOccupied() == true);
  TEST_CHECK(h.getBucket(3)->getBitmapIndex(0) == false);

  TEST_CHECK(h.getBucket(0)->getTuples().getRoot()->mytuple == &c);
  TEST_CHECK(h.getBucket(0)->getOccupied() == true);
  TEST_CHECK(h.getBucket(0)->getBitmapIndex(0) == false);
}

/* Full  HT Insert
 * -------------------
 * The entirety of the HT is occupied
 * Rehash is needed
 */
void test_HTinsert4() {
  hashTable h(32);

  tuple tuples[40];

  for (int64_t i = 0; i < 40; i++) {
    tuples[i] = {std::rand(), std::rand()};
    h.insert(&tuples[i]);
  }
}

TEST_LIST = {{"test_partinioning_fn", test_partitioning_function},
             {"test_part_pass1", test_partitions_1},
             {"test_part_pass2", test_partitions_2},
             {"HT's Hash Function", test_HThash2},
             {"Normal HT Insert", test_HTinsert1},
             {"Same Key HT Insert", test_HTinsert2},
             {"Empty NBHD Slot HT Insert", test_HTinsert3},
             {"Full NBHD HT Insert", test_HTinsert4},

             {NULL, NULL}};