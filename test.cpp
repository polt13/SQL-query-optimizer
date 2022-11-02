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

  bool allOccupiedBefore = true;
  bool allOccupiedAfter = true;

  for (int64_t i = 0; i < 40; i++) {
    tuples[i] = {keys[key_index++], std::rand()};
    h.insert(&tuples[i]);

    // Check if after the 34 the HT is full
    if (i == 33)
      for (int64_t j = 0; j < 34; j++)
        if (h.getBucket(j)->getOccupied() == false) {
          allOccupiedBefore = false;
          break;
        }

    // Check if after the 35 the HT is NOT full
    if (i == 34)
      for (int64_t j = 0; j < 69; j++)
        if (h.getBucket(j)->getOccupied() == false) {
          allOccupiedAfter = false;
          break;
        }
  }

  // check that all buckets show as occupied after the 34th insertion
  // but after the 35th insertion (rehash) they do not
  TEST_CHECK(allOccupiedBefore == true);
  TEST_CHECK(allOccupiedAfter == false);
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

  bool allOccupied = true;
  bool rightOrder = true;

  for (int64_t i = 0; i < 40; i++) {
    tuples[i] = {key, std::rand()};  // hash in same bucket

    h.insert(&tuples[i]);

    // check if after the 32 all neighborhood is full
    if (i == 31) {
      int b_index = 36;
      int t_index = 0;
      bucket* b = h.getBucket(b_index);
      for (int j = 0; j < NBHD_SIZE; j++) {
        if (b->getBitmapIndex(j) == false) allOccupied = false;
        if (h.getBucket(b_index % h.getBucketCount())
                ->getTuples()
                .getRoot()
                ->mytuple != &tuples[t_index++])
          rightOrder = false;
        b_index++;
      }
    }

    key += 50;
  }

  // check that all the bitmap indexes show as occupied after the 32nd
  // insertion, and that the tuples have been inserted in the right order
  TEST_CHECK(allOccupied == true);
  TEST_CHECK(rightOrder == true);
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

void test_build_1() {
  int keys[] = {636,   1088,  1381,  1634,  1755,  2063,  2394,  2456,  2932,
                3359,  3384,  3772,  3852,  4015,  4961,  5459,  6445,  6780,
                7071,  7318,  8552,  9469,  10216, 10515, 11359, 11460, 11574,
                11714, 12033, 13043, 13203, 13214, 13232, 13705, 13957, 14808,
                15476, 15931, 16025, 16363, 16951, 17133, 17419, 18089, 18143,
                18234, 18248, 18933, 19039, 20237};

  tuple* tuples = new tuple[100];
  tuple* tuples2 = new tuple[200];

  for (int64_t i = 0; i < 100; i++)
    tuples[i] = {keys[std::rand() % 50], std::rand()};

  for (int64_t i = 0; i < 200; i++)
    tuples2[i] = {keys[std::rand() % 50], std::rand()};

  relation r(tuples, 100);
  relation s(tuples2, 200);

  // partitioner maintains internal state - histogram, psum.. create a diff. one
  // for each relation
  Partitioner rp, sp;

  relation r_ = rp.partition(r);
  int64_t forcePartitioning = rp.getPartitioningLevel();

  relation s_ = sp.partition(s, forcePartitioning);

  // if no partitioning done - no histogram generated...
  // this depends on L2_SIZE
  if (forcePartitioning != 0) {
    Histogram* histr = rp.getHistogram();
    Histogram* hists = sp.getHistogram();

    // check if both relations have the same number of partitions
    // the two test_checks are equivalent
    TEST_CHECK(histr->getSize() == hists->getSize());
  }

  TEST_CHECK(rp.getPartitioningLevel() == sp.getPartitioningLevel());

  Partitioner rp2, sp2;
  relation r_2 = rp2.partition(r, 1);
  forcePartitioning = rp2.getPartitioningLevel();

  relation s_2 = sp2.partition(s, 1);

  Histogram* histr2 = rp2.getHistogram();
  Histogram* hists2 = sp2.getHistogram();

  TEST_CHECK(histr2->getSize() == hists2->getSize());
  TEST_CHECK(
      (rp2.getPartitioningLevel() == 1L && 1L == sp2.getPartitioningLevel()));

  Partitioner rp3, sp3;
  relation r_3 = rp3.partition(r, 2);
  forcePartitioning = rp3.getPartitioningLevel();

  relation s_3 = sp3.partition(s, 2);

  Histogram* histr3 = rp3.getHistogram();
  Histogram* hists3 = sp3.getHistogram();

  TEST_CHECK(histr3->getSize() == hists3->getSize());
  TEST_CHECK((rp3.getPartitioningLevel() == 2L) &&
             (2L == sp3.getPartitioningLevel()));
}

void test_build_2() {
  int keys[] = {636,   1088,  1381,  1634,  1755,  2063,  2394,  2456,  2932,
                3359,  3384,  3772,  3852,  4015,  4961,  5459,  6445,  6780,
                7071,  7318,  8552,  9469,  10216, 10515, 11359, 11460, 11574,
                11714, 12033, 13043, 13203, 13214, 13232, 13705, 13957, 14808,
                15476, 15931, 16025, 16363, 16951, 17133, 17419, 18089, 18143,
                18234, 18248, 18933, 19039, 20237};

  tuple* tuples = new tuple[100];
  tuple* tuples2 = new tuple[200];

  for (int64_t i = 0; i < 100; i++)
    tuples[i] = {keys[std::rand() % 50], std::rand()};

  for (int64_t i = 0; i < 200; i++) {
    if (i == 2 || i == 55 || i == 91 || i == 98) {
      tuples2[i] = tuples[i];
    } else
      tuples2[i] = {0, std::rand()};
  }

  relation r(tuples, 100);
  relation s(tuples2, 200);

  // partitioner maintains internal state - histogram, psum.. create a diff. one
  // for each relation
  Partitioner rp, sp;

  relation r_ = rp.partition(r, 2);

  relation s_ = sp.partition(s, 2);

  Histogram* histr = rp.getHistogram();
  Histogram* hists = sp.getHistogram();
  int64_t partitions = histr->getSize();

  const int64_t* rpsum = histr->getPsum();

  hashTable** pht = new hashTable* [partitions] {};

  for (int64_t i = 0; i < partitions; i++) {
    int64_t entries = (*histr)[i];
    pht[i] = new hashTable(entries);

    int64_t start = rpsum[i];
    std::printf("Partition starts at %ld\n", start);
    int64_t end =
        (i < (partitions - 1)) ? (rpsum[i + 1]) : (r_.getAmount() - 1);

    for (int64_t j = start; j < end; j++) pht[i]->insert(&r_[j]);
  }

  for (int64_t i = 0; i < 100; i++) {
    int64_t index = Partitioner::hash1(r[i].getKey(), USE_BITS_NEXT);
    // check if the element is properly inserted
    TEST_CHECK(pht[index]->findEntry(r[i].getKey()) != nullptr);
  }

  int64_t matchedTuples = 0;
  for (int64_t i = 0; i < 200; i++) {
    int64_t index = Partitioner::hash1(s[i].getKey(), USE_BITS_NEXT);
    List* item_exists = pht[index]->findEntry(s[i].getKey());
    if (item_exists) matchedTuples++;
  }

  // only 4 of S keys exist in R
  TEST_CHECK(matchedTuples == 4);

  for (int64_t i = 0; i < partitions; i++) delete pht[i];

  delete[] pht;
}

// no partitioning
// this test will FAIL if the L2_SIZE is extremely small
void test_build_3() {}

TEST_LIST = {{"Partitioning function", test_partitioning_function},
             {"Partitioning - pass 1", test_partitions_1},
             {"Partitioning - pass 2", test_partitions_2},
             {"HT's Hash Function", test_HThash2},
             {"Normal HT Insert", test_HTinsert1},
             {"Same Key HT Insert", test_HTinsert2},
             {"Empty NBHD Slot HT Insert", test_HTinsert3},
             {"Full HT Insert", test_HTinsert4},
             {"Full NBHD HT Insert", test_HTinsert5},
             {"Swap HT Insert", test_HTinsert6},
             {"None for Swap HT Insert", test_HTinsert7},
             {"Relations have equal amount of partitions", test_build_1},
             {"Partition's HT has entry", test_build_2},
             {"No partitioning", test_build_3},
             {NULL, NULL}};