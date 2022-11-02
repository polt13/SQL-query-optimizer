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
  tuple a{10, 1};  // 10 % 40 = 10
  hashTable h(40);

  h.insert(&a);
  TEST_CHECK(h.getBucket(10)->getTuples().getRoot()->mytuple == &a);
  TEST_CHECK(h.getBucket(10)->getOccupied() == true);
  TEST_CHECK(h.getBucket(10)->getBitmapIndex(0) == true);
}

/* Same Key HT Insert
 * ------------------
 * Two tuples with same Key but different Payload (rowID)
 * Second tuple has to be inserted (append) into first tuple's List
 */
void test_HTinsert2() {
  tuple a{10, 1};  // 10 % 40 = 10
  tuple b{10, 2};  // Same key but different row

  hashTable h(40);

  h.insert(&a);
  h.insert(&b);
  TEST_CHECK(h.getBucket(10)->getTuples().getRoot()->mytuple == &a);
  TEST_CHECK(h.getBucket(10)->getOccupied() == true);
  TEST_CHECK(h.getBucket(10)->getBitmapIndex(0) == true);
  TEST_CHECK(h.getBucket(10)->getTuples().find(b) == true);
}

/* Empty NBHD Slot HT Insert
 * -------------------------
 * Index "hashVal" already Occupied, find empty slot
 * Empty slot already in NBHD_SIZE range, no need to swap
 * with anything. Just insert in index j instead.
 * Includes the case when exceeding HT's Size (start from 0)
 */
void test_HTinsert3() {
  tuple a{38, 1};   // 38 % 40 = 38
  tuple b{78, 3};   // 78 % 40 = 38
  tuple c{118, 6};  // 118 % 40 = 38

  hashTable h(40);

  h.insert(&a);
  h.insert(&b);
  h.insert(&c);
  TEST_CHECK(h.getBucket(38)->getTuples().getRoot()->mytuple == &a);
  TEST_CHECK(h.getBucket(38)->getOccupied() == true);
  TEST_CHECK(h.getBucket(38)->getBitmapIndex(0) == true);
  TEST_CHECK(h.getBucket(38)->getBitmapIndex(1) == true);
  TEST_CHECK(h.getBucket(38)->getBitmapIndex(2) == true);

  TEST_CHECK(h.getBucket(39)->getTuples().getRoot()->mytuple == &b);
  TEST_CHECK(h.getBucket(39)->getOccupied() == true);
  TEST_CHECK(h.getBucket(39)->getBitmapIndex(0) == false);

  TEST_CHECK(h.getBucket(0)->getTuples().getRoot()->mytuple == &c);
  TEST_CHECK(h.getBucket(0)->getOccupied() == true);
  TEST_CHECK(h.getBucket(0)->getBitmapIndex(0) == false);
}

/* Full HT Insert
 * --------------
 * The entirety of the HT is occupied
 * Rehash is needed
 */
void test_HTinsert4() {
  int keys[] = {636,   1088,  1381,  1634,  1755,  2063,  2394,  2456,  2932,
                3359,  3384,  3772,  3852,  4015,  4961,  5459,  6445,  6780,
                7071,  7318,  8552,  9469,  10216, 10515, 11359, 11460, 11574,
                11714, 12033, 13043, 13203, 13214, 13232, 13705, 13957, 14808,
                15476, 15931, 16025, 16363, 16951, 17133, 17419, 18089, 18143,
                18234, 18248, 18933, 19039, 20237};

  tuple tuples[40];

  hashTable h(34);

  int64_t key_index = 0;

  for (int64_t i = 0; i < 40; i++) {
    tuples[i] = {keys[key_index++], std::rand()};
    h.insert(&tuples[i]);
  }
}

/* Full NBHD HT Insert
 * -------------------
 * The entirety of a specific Bucket's Neighbourhood is occupied
 * Rehash is needed
 */
void test_HTinsert5() {
  int key = 636;  // 636 % 50 = 36

  tuple tuples[40];

  hashTable h(50);

  for (int64_t i = 0; i < 40; i++) {
    tuples[i] = {key, std::rand()};  // hash in same bucket
    h.insert(&tuples[i]);
    key += 50;
  }
}

/* Swap HT Insert
 * --------------
 * Index "hashVal" already Occupied, find empty slot
 * Empty slot NOT in NBHD_SIZE range, need to swap elements
 * in order to "create" empty slot (by swapping)
 * which is in NBHD_SIZE range
 * Includes the case when exceeding HT's Size (start from 0)
 */
void test_HTinsert6() {
  int key = 26;  // 26 % 40 = 26

  tuple tuples[35];

  hashTable h(40);

  for (int64_t i = 0; i < 35; i++) {
    if (i == 32) key = 66;  // 66 % 40 = 26
    tuples[i] = {key, std::rand()};
    h.insert(&tuples[i]);
    key++;
  }

  TEST_CHECK(h.getBucket(26)->getTuples().getRoot()->mytuple == &tuples[0]);
  TEST_CHECK(h.getBucket(26)->getOccupied() == true);
  TEST_CHECK(h.getBucket(26)->getBitmapIndex(0) == true);
  TEST_CHECK(h.getBucket(26)->getBitmapIndex(1) == true);
  TEST_CHECK(h.getBucket(27)->getTuples().getRoot()->mytuple == &tuples[32]);
  TEST_CHECK(h.getBucket(27)->getOccupied() == true);
  TEST_CHECK(h.getBucket(27)->getBitmapIndex(0) == false);
  TEST_CHECK(h.getBucket(27)->getBitmapIndex(1) == true);
  TEST_CHECK(h.getBucket(27)->getBitmapIndex(31) == true);
  TEST_CHECK(h.getBucket(28)->getTuples().getRoot()->mytuple == &tuples[33]);
  TEST_CHECK(h.getBucket(28)->getOccupied() == true);
  TEST_CHECK(h.getBucket(28)->getBitmapIndex(0) == false);
  TEST_CHECK(h.getBucket(28)->getBitmapIndex(1) == true);
  TEST_CHECK(h.getBucket(28)->getBitmapIndex(31) == true);
  TEST_CHECK(h.getBucket(29)->getTuples().getRoot()->mytuple == &tuples[34]);
  TEST_CHECK(h.getBucket(29)->getOccupied() == true);
  TEST_CHECK(h.getBucket(29)->getBitmapIndex(0) == false);
  TEST_CHECK(h.getBucket(29)->getBitmapIndex(31) == true);
}

/* None for Swap HT Insert
 * -----------------------
 * Index "hashVal" already Occupied, find empty slot
 * Empty slot NOT in NBHD_SIZE range, need to swap elements
 * BUT no element to fullfil the requirements is found
 * Rehash is needed
 * Includes the case when exceeding HT's Size (start from 0)
 */
void test_HTinsert7() {
  int key = 26;  // 26 % 40 = 26

  tuple tuples[34];

  hashTable h(40);

  for (int64_t i = 0; i < 34; i++) {
    if (i == 32) key = 27;  // 27 % 40 = 27
    tuples[i] = {key, std::rand()};
    h.insert(&tuples[i]);
    key += 40;
  }

  // key = 26 | hashVal = 26 % 81 = 26
  TEST_CHECK(h.getBucket(26)->getTuples().getRoot()->mytuple == &tuples[0]);
  TEST_CHECK(h.getBucket(26)->getOccupied() == true);
  TEST_CHECK(h.getBucket(26)->getBitmapIndex(0) == true);

  // key = 66 | hashVal = 66 % 81 = 66
  TEST_CHECK(h.getBucket(66)->getTuples().getRoot()->mytuple == &tuples[1]);
  TEST_CHECK(h.getBucket(66)->getOccupied() == true);
  TEST_CHECK(h.getBucket(66)->getBitmapIndex(0) == true);

  // key = 106 | hashVal = 106 % 81 = 25
  TEST_CHECK(h.getBucket(25)->getTuples().getRoot()->mytuple == &tuples[2]);
  TEST_CHECK(h.getBucket(25)->getOccupied() == true);
  TEST_CHECK(h.getBucket(25)->getBitmapIndex(0) == true);

  // key = 146 | hashVal = 146 % 81 = 66
  TEST_CHECK(h.getBucket(65)->getTuples().getRoot()->mytuple == &tuples[3]);
  TEST_CHECK(h.getBucket(65)->getOccupied() == true);
  TEST_CHECK(h.getBucket(65)->getBitmapIndex(0) == true);

  // Pattern...
  // ................. and so on ...........

  // key = 1226 | hashVal = 1226 % 81 = 11
  TEST_CHECK(h.getBucket(11)->getTuples().getRoot()->mytuple == &tuples[30]);
  TEST_CHECK(h.getBucket(11)->getOccupied() == true);
  TEST_CHECK(h.getBucket(11)->getBitmapIndex(0) == true);

  // key = 1266 | hashVal = 1266 % 81 = 51
  TEST_CHECK(h.getBucket(51)->getTuples().getRoot()->mytuple == &tuples[31]);
  TEST_CHECK(h.getBucket(51)->getOccupied() == true);
  TEST_CHECK(h.getBucket(51)->getBitmapIndex(0) == true);

  // key = 27 | hashVal = 27 % 81 = 27
  TEST_CHECK(h.getBucket(27)->getTuples().getRoot()->mytuple == &tuples[32]);
  TEST_CHECK(h.getBucket(27)->getOccupied() == true);
  TEST_CHECK(h.getBucket(27)->getBitmapIndex(0) == true);

  // key = 67 | hashVal = 67 % 81 = 67
  TEST_CHECK(h.getBucket(67)->getTuples().getRoot()->mytuple == &tuples[33]);
  TEST_CHECK(h.getBucket(67)->getOccupied() == true);
  TEST_CHECK(h.getBucket(67)->getBitmapIndex(0) == true);
}

TEST_LIST = {{"test_partinioning_fn", test_partitioning_function},
             {"test_part_pass1", test_partitions_1},
             {"test_part_pass2", test_partitions_2},
             {"HT's Hash Function", test_HThash2},
             {"Normal HT Insert", test_HTinsert1},
             {"Same Key HT Insert", test_HTinsert2},
             {"Empty NBHD Slot HT Insert", test_HTinsert3},
             {"Full HT Insert", test_HTinsert4},
             {"Full NBHD HT Insert", test_HTinsert5},
             {"Swap HT Insert", test_HTinsert6},
             {"None for Swap HT Insert", test_HTinsert7},
             {NULL, NULL}};