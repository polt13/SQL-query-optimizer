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

// WORKS WITH USE_BITS=2
void test_partitions_3() {
  int64_t keys[] = {418,  690,  1536, 160,  1552, 1312, 1538, 320, 1537,
                    1281, 1282, 1602, 1138, 274,  978,  561,  99,  1008,
                    480,  144,  1264, 83,   130,  385,  1424, 674};

  int64_t expectedOrder[] = {1536, 160,  1552, 1312, 320, 1008, 480, 144,  1264,
                             1424, 1537, 1281, 561,  385, 418,  690, 1538, 1282,
                             1602, 1138, 274,  978,  130, 674,  99,  83};

  tuple* tuples = new tuple[26];
  for (int64_t i = 0; i < 26; i++) tuples[i] = {keys[i], std::rand()};

  relation r(tuples, 26);

  Partitioner rp;
  relation r_ = rp.partition(r, 1);
  Histogram* hist = rp.getHistogram();
  const int64_t* psum = hist->getPsum();

  TEST_CHECK((*hist)[0] == 10);
  TEST_CHECK(psum[0] == 0);

  TEST_CHECK((*hist)[1] == 4);
  TEST_CHECK(psum[1] == 10);

  TEST_CHECK((*hist)[2] == 10);
  TEST_CHECK(psum[2] == 14);

  TEST_CHECK((*hist)[3] == 2);
  TEST_CHECK(psum[3] == 24);

  for (int64_t i = 0; i < 26; i++) {
    TEST_CHECK(r_[i].getKey() == expectedOrder[i]);
  }
}

// works only with USE_BITS_NEXT == 4
void test_partitions_4() {
  int64_t keys[] = {657,  1237, 1060, 1154, 832,  1417, 456,  449,  112,  728,
                    1140, 1402, 583,  1366, 1599, 1447, 1185, 1466, 1097, 101,
                    941,  888,  1090, 1147, 865,  1333, 1354, 1264, 1415, 681,
                    1058, 1169, 1531, 782,  1386, 1033, 1185, 1254, 1555, 256,
                    775,  484,  1394, 987,  1197, 377,  1011, 644,  27,   317,
                    182,  1083, 427,  1244, 202,  835,  195,  698,  55,   742,
                    1105, 329,  524,  1270, 1136, 598,  244,  1035, 1015, 838,
                    420,  283,  1511, 1262, 695,  610,  1417, 1525, 1349, 1429,
                    901,  582,  955,  802,  924,  1586, 254,  1094, 601,  949,
                    1441, 715,  900,  1022, 1237, 162,  1575, 790,  740};

  int64_t expectedOrder[] = {
      832,  112,  1264, 256,  1136, 657,  449,  1185, 865,  1169, 1185,
      1105, 1441, 1154, 1090, 1058, 1394, 610,  802,  1586, 162,  1555,
      1011, 835,  195,  1060, 1140, 484,  644,  244,  420,  900,  740,
      1237, 101,  1333, 1525, 1349, 1429, 901,  949,  1237, 1366, 1254,
      182,  742,  1270, 598,  838,  582,  1094, 790,  583,  1447, 1415,
      775,  55,   1015, 1511, 695,  1575, 456,  728,  888,  1417, 1097,
      681,  1033, 377,  329,  1417, 601,  1402, 1466, 1354, 1386, 202,
      698,  1147, 1531, 987,  27,   1083, 427,  1035, 283,  955,  715,
      1244, 524,  924,  941,  1197, 317,  782,  1262, 254,  1022, 1599};

  tuple* tuples = new tuple[99];
  for (int64_t i = 0; i < 99; i++) tuples[i] = {keys[i], std::rand()};

  relation r(tuples, 99);

  Partitioner rp;
  relation r_ = rp.partition(r, 2);
  Histogram* hist = rp.getHistogram();
  const int64_t* psum = hist->getPsum();

  TEST_CHECK((*hist)[0] == 5);
  TEST_CHECK(psum[0] == 0);

  TEST_CHECK((*hist)[1] == 8);
  TEST_CHECK(psum[1] == 5);

  TEST_CHECK((*hist)[2] == 8);
  TEST_CHECK(psum[2] == 13);

  TEST_CHECK((*hist)[3] == 4);
  TEST_CHECK(psum[3] == 21);

  TEST_CHECK((*hist)[4] == 8);
  TEST_CHECK(psum[4] == 25);

  TEST_CHECK((*hist)[5] == 9);
  TEST_CHECK(psum[5] == 33);

  TEST_CHECK((*hist)[6] == 10);
  TEST_CHECK(psum[6] == 42);

  TEST_CHECK((*hist)[7] == 9);
  TEST_CHECK(psum[7] == 52);

  TEST_CHECK((*hist)[8] == 3);
  TEST_CHECK(psum[8] == 61);

  TEST_CHECK((*hist)[9] == 8);
  TEST_CHECK(psum[9] == 64);

  TEST_CHECK((*hist)[10] == 6);
  TEST_CHECK(psum[10] == 72);

  TEST_CHECK((*hist)[11] == 10);
  TEST_CHECK(psum[11] == 78);

  TEST_CHECK((*hist)[12] == 3);
  TEST_CHECK(psum[12] == 88);

  TEST_CHECK((*hist)[13] == 3);
  TEST_CHECK(psum[13] == 91);

  TEST_CHECK((*hist)[14] == 4);
  TEST_CHECK(psum[14] == 94);

  TEST_CHECK((*hist)[15] == 1);
  TEST_CHECK(psum[15] == 98);

  for (int64_t i = 0; i < 99; i++) {
    TEST_CHECK(r_[i].getKey() == expectedOrder[i]);
  }
}

// Test will fail if CACHE is set to an EXTREMELY small size
void test_no_partition() {
  tuple a{0, 5};
  tuple b{3, 2};
  tuple c{4, 5};
  tuple* tuples = new tuple[3]{a, b, c};
  relation r(tuples, 3);
  Partitioner rp;
  relation r_ = rp.partition(r);
  TEST_CHECK(rp.getPartitioningLevel() == 0);
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

/* Relations have equal amount of partitions
 * -----------------------------------------
 */
void test_eq_partitions() {
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
  Partitioner rp2, sp2;
  relation r_2 = rp2.partition(r, 1);
  relation s_2 = sp2.partition(s, 1);

  Histogram* histr2 = rp2.getHistogram();
  Histogram* hists2 = sp2.getHistogram();

  TEST_CHECK(histr2->getSize() == hists2->getSize());
  TEST_CHECK(
      (rp2.getPartitioningLevel() == 1L && 1L == sp2.getPartitioningLevel()));

  Partitioner rp3, sp3;
  relation r_3 = rp3.partition(r, 2);

  relation s_3 = sp3.partition(s, 2);

  Histogram* histr3 = rp3.getHistogram();
  Histogram* hists3 = sp3.getHistogram();

  TEST_CHECK(histr3->getSize() == hists3->getSize());
  TEST_CHECK((rp3.getPartitioningLevel() == 2L) &&
             (2L == sp3.getPartitioningLevel()));
}

/* Partition's HT has entry
 * ------------------------
 */
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
  // Histogram* hists = sp.getHistogram();
  int64_t partitions = histr->getSize();

  const int64_t* rpsum = histr->getPsum();

  hashTable** pht = new hashTable* [partitions] {};

  for (int64_t i = 0; i < partitions; i++) {
    int64_t entries = (*histr)[i];
    pht[i] = new hashTable(entries);

    int64_t start = rpsum[i];
    // std::printf("Partition starts at %ld\n", start);
    int64_t end = (i < (partitions - 1)) ? (rpsum[i + 1]) : (r_.getAmount());

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

/* Test Join with Partitioning (Pass 1)
 * ------------------------------------
 */
void test_join_1() {
  tuple* tuples1 = new tuple[6]{{5, 1}, {3, 2}, {3, 3}, {2, 4}, {6, 5}, {1, 5}};
  tuple* tuples2 = new tuple[3]{{3, 10}, {3, 11}, {1, 12}};

  relation r(tuples1, 6);
  relation s(tuples2, 3);
  result t = PartitionedHashJoin(r, s, 1, 4, 8);
  /* for (int64_t i = 0; i < t.result_size; i++) {
    std::printf("\nr_id: %ld, r_row: %ld\ns_id: %ld, s_row: %ld\n",
                t[i].a.getKey(), t[i].a.getPayload(), t[i].b.getKey(),
                t[i].b.getPayload());
  } */
  TEST_CHECK(t.result_size == 5);

  // a = {1, 5} | b = {1, 12}
  TEST_CHECK(t[0].a.getKey() == r[5].getKey());
  TEST_CHECK(t[0].a.getPayload() == 5);
  TEST_CHECK(t[0].b.getKey() == s[2].getKey());
  TEST_CHECK(t[0].b.getPayload() == 12);

  // a = {3, 2} | b = {3, 10}
  TEST_CHECK(t[1].a.getKey() == r[1].getKey());
  TEST_CHECK(t[1].a.getPayload() == 2);
  TEST_CHECK(t[1].b.getKey() == s[0].getKey());
  TEST_CHECK(t[1].b.getPayload() == 10);

  // a = {3, 3} | b = {3, 10}
  TEST_CHECK(t[2].a.getKey() == r[2].getKey());
  TEST_CHECK(t[2].a.getPayload() == 3);
  TEST_CHECK(t[2].b.getKey() == s[0].getKey());
  TEST_CHECK(t[2].b.getPayload() == 10);

  // a = {3, 2} | b = {3, 11}
  TEST_CHECK(t[3].a.getKey() == r[1].getKey());
  TEST_CHECK(t[3].a.getPayload() == 2);
  TEST_CHECK(t[3].b.getKey() == s[1].getKey());
  TEST_CHECK(t[3].b.getPayload() == 11);

  // a = {3, 3} | b = {3, 11}
  TEST_CHECK(t[4].a.getKey() == r[2].getKey());
  TEST_CHECK(t[4].a.getPayload() == 3);
  TEST_CHECK(t[4].b.getKey() == s[1].getKey());
  TEST_CHECK(t[4].b.getPayload() == 11);
}

/* Test Join with Partitioning (Pass 2)
 * ------------------------------------
 */
void test_join_2() {
  tuple* tuples1 = new tuple[6]{{5, 1}, {5, 2}, {3, 3}, {2, 4}, {6, 5}, {1, 5}};
  tuple* tuples2 = new tuple[3]{{3, 10}, {4, 11}, {1, 12}};

  relation r(tuples1, 6);
  relation s(tuples2, 3);
  result t = PartitionedHashJoin(r, s, 2, 4, 8);
  /* for (int64_t i = 0; i < t.result_size; i++) {
    std::printf("\nr_id: %ld, r_row: %ld\ns_id: %ld, s_row: %ld\n",
                t[i].a.getKey(), t[i].a.getPayload(), t[i].b.getKey(),
                t[i].b.getPayload());
  } */
  // TEST_CHECK(t.result_size == 2);
  /*
    // a = {1, 5} | b = {1, 12}
    TEST_CHECK(t[0].a.getKey() == r[5].getKey());
    TEST_CHECK(t[0].a.getPayload() == 5);
    TEST_CHECK(t[0].b.getKey() == s[2].getKey());
    TEST_CHECK(t[0].b.getPayload() == 12);

    // a = {3, 3} | b = {3, 10}
    TEST_CHECK(t[1].a.getKey() == r[0].getKey());
    TEST_CHECK(t[1].a.getPayload() == 3);
    TEST_CHECK(t[1].b.getKey() == s[0].getKey());
    TEST_CHECK(t[1].b.getPayload() == 10); */
}

/* Test Join without Partitioning
 * ------------------------------
 */
void test_join_3() {
  tuple* tuples1 = new tuple[6]{{5, 1}, {3, 2}, {3, 3}, {2, 4}, {6, 5}, {1, 5}};
  tuple* tuples2 = new tuple[3]{{3, 10}, {3, 11}, {1, 12}};

  relation r(tuples1, 6);
  relation s(tuples2, 3);
  result t = PartitionedHashJoin(r, s, 2, 4, 8);
  // for (int64_t i = 0; i < t.result_size; i++) {
  //   std::printf("\nr_id: %ld, s_id: %ld\n", t[i].a.getKey(),
  //   t[i].b.getKey());
  // }
}

void test_join_4() {
  int64_t keys1[] = {93, 99, 56, 55, 45, 90, 13, 74, 22, 31, 19, 99, 13, 48, 43,
                     21, 84, 95, 7,  96, 22, 11, 46, 56, 39, 57, 78, 30, 31, 18,
                     74, 3,  72, 2,  40, 69, 69, 79, 58, 3,  60, 15, 79, 68, 50,
                     75, 39, 7,  77, 9,  75, 67, 18, 33, 81, 4,  12, 90, 4,  20,
                     56, 5,  66, 71, 21, 30, 59, 71, 89, 3,  40, 61, 86, 7,  49,
                     17, 76, 20, 36, 11, 4,  98, 36, 80, 34, 9,  54, 54, 72, 19,
                     68, 31, 44, 90, 49, 54, 52, 26, 30, 19};
  int64_t keys2[] = {
      28, 7,  39, 69, 24, 44, 92, 78, 76, 61, 47, 29, 1,  35, 49, 54, 41,
      51, 41, 1,  1,  37, 42, 45, 22, 2,  81, 9,  9,  33, 74, 59, 52, 54,
      72, 7,  80, 49, 4,  7,  32, 29, 40, 23, 51, 46, 92, 37, 64, 82, 38,
      63, 10, 55, 22, 28, 9,  30, 21, 68, 34, 22, 71, 55, 81, 86, 26, 38,
      95, 50, 29, 29, 6,  18, 26, 12, 87, 89, 35, 40, 9,  63, 90, 42, 41,
      77, 25, 60, 26, 15, 7,  24, 79, 87, 4,  74, 59, 4,  25, 50, 15, 96,
      17, 43, 20, 4,  35, 79, 52, 4,  29, 44, 71, 36, 69, 36, 36, 19, 52,
      34, 96, 54, 36, 47, 98, 63, 89, 47, 16, 47, 23, 37, 78, 63, 50, 1,
      42, 96, 58, 78, 54, 64, 47, 7,  20, 27, 80, 64, 19, 67, 32, 39, 69,
      32, 31, 76, 42, 26, 88, 67, 81, 90, 64, 70, 39, 13, 12, 15, 93, 9,
      45, 93, 85, 94, 61, 29, 48, 76, 96, 32, 97, 89, 68, 49, 19, 22, 39,
      68, 14, 7,  30, 44, 28, 57, 66, 34, 97, 72, 19, 88};

  tuple* tuples1 = new tuple[100];
  tuple* tuples2 = new tuple[200];
  for (int64_t i = 0; i < 100; i++) {
    tuples1[i] = {keys1[i], keys1[i]};
  }
  for (int64_t i = 0; i < 200; i++) {
    tuples2[i] = {keys2[i], keys2[i]};
  }

  relation r(tuples1, 100);
  relation s(tuples2, 200);

  result t = PartitionedHashJoin(r, s);
  TEST_CHECK(t.result_size == 219);
}

TEST_LIST = {
    {"Partitioning function", test_partitioning_function},
    {"Partitioning - small test   (partitioning level 1)", test_partitions_1},
    {"Partitioning - small test 2 (partition level 2)", test_partitions_2},
    {"Partitioning - big test (partitioning level 1)", test_partitions_3},
    {"Partitioning - big test (partitioning level 2)", test_partitions_4},
    {"Test No Partitioning", test_no_partition},
    {"HT's Hash Function", test_HThash2},
    {"Normal HT Insert", test_HTinsert1},
    {"Same Key HT Insert", test_HTinsert2},
    {"Empty NBHD Slot HT Insert", test_HTinsert3},
    {"Full HT Insert", test_HTinsert4},
    {"Full NBHD HT Insert", test_HTinsert5},
    {"Swap HT Insert", test_HTinsert6},
    {"None for Swap HT Insert", test_HTinsert7},
    {"Relations have equal amount of partitions", test_eq_partitions},
    {"Partition's HT has entry", test_build_1},
    {"Join with Partitioning (Pass 1)", test_join_1},
    {"Join with Partitioning (Pass 2)", test_join_2},
    {"Join without Partitioning", test_join_3},
    {"Join Big", test_join_4},

    {NULL, NULL}};