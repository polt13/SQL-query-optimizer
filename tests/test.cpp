#include "acutest.h"
#include "dataForm.h"
#include "hashTable.h"
#include "histogram.h"
#include "list.h"
#include "partitioner.h"
#include "simple_vector.h"
#include "simple_queue.h"

// use only for unit testing
result PartitionedHashJoin_ST(relation&, relation&, int64_t = -1,
                              int64_t = USE_BITS, int64_t = USE_BITS_NEXT);

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

/* Test Join with Partitioning (One Pass)
 * --------------------------------------
 */
void test_join_1() {
  tuple* tuples1 = new tuple[6]{{5, 1}, {3, 2}, {3, 3}, {2, 4}, {6, 5}, {1, 6}};
  tuple* tuples2 = new tuple[3]{{3, 10}, {3, 11}, {1, 12}};

  relation r(tuples1, 6);
  relation s(tuples2, 3);

  result t = PartitionedHashJoin_ST(r, s, 1, 4, 8);

  TEST_CHECK(t.getSize() == 5);

  // a = {1, 6} | b = {1, 12}

  // TEST_CHECK(t[0].a.getKey() == r[5].getKey());
  // TEST_CHECK(t[0].b.getKey() == s[2].getKey());

  TEST_CHECK(t[0].rowid_1 == 6);
  TEST_CHECK(t[0].rowid_2 == 12);

  // // a = {3, 2} | b = {3, 10}
  // TEST_CHECK(t[1].a.getKey() == r[1].getKey());
  // TEST_CHECK(t[1].b.getKey() == s[0].getKey());
  TEST_CHECK(t[1].rowid_1 == 2);
  TEST_CHECK(t[1].rowid_2 == 10);

  // a = {3, 3} | b = {3, 10}
  // TEST_CHECK(t[2].a.getKey() == r[2].getKey());
  // TEST_CHECK(t[2].b.getKey() == s[0].getKey());

  TEST_CHECK(t[2].rowid_1 == 3);
  TEST_CHECK(t[2].rowid_2 == 10);

  // a = {3, 2} | b = {3, 11}
  // TEST_CHECK(t[3].a.getKey() == r[1].getKey());
  // TEST_CHECK(t[3].b.getKey() == s[1].getKey());

  TEST_CHECK(t[3].rowid_1 == 2);
  TEST_CHECK(t[3].rowid_2 == 11);

  // a = {3, 3} | b = {3, 11}
  // TEST_CHECK(t[4].a.getKey() == r[2].getKey());
  //  TEST_CHECK(t[4].b.getKey() == s[1].getKey());
  TEST_CHECK(t[4].rowid_1 == 3);
  TEST_CHECK(t[4].rowid_2 == 11);
}

/* Test Join with Partitioning (Two Pass)
 * --------------------------------------
 */
void test_join_2() {
  tuple* tuples1 = new tuple[6]{{5, 1}, {5, 2}, {3, 3}, {2, 4}, {4, 5}, {1, 6}};
  tuple* tuples2 = new tuple[3]{{3, 10}, {4, 11}, {1, 12}};

  relation r(tuples1, 6);
  relation s(tuples2, 3);
  result t = PartitionedHashJoin_ST(r, s, 2, 4, 8);
  /* for (int64_t i = 0; i < t.result_size; i++) {
    std::printf("\nr_id: %ld, r_row: %ld\ns_id: %ld, s_row: %ld\n",
                t[i].a.getKey(), t[i].a.getPayload(), t[i].b.getKey(),
                t[i].b.getPayload());
  } */
  TEST_CHECK(t.getSize() == 3);

  // a = {1, 6} | b = {1, 12}
  // TEST_CHECK(t[0].a.getKey() == r[5].getKey());
  //   TEST_CHECK(t[0].b.getKey() == s[2].getKey());

  TEST_CHECK(t[0].rowid_1 == 6);
  TEST_CHECK(t[0].rowid_2 == 12);

  // a = {3, 3} | b = {3, 10}
  // TEST_CHECK(t[1].a.getKey() == r[2].getKey());
  // TEST_CHECK(t[1].b.getKey() == s[0].getKey());

  TEST_CHECK(t[1].rowid_1 == 3);
  TEST_CHECK(t[1].rowid_2 == 10);

  // a = {4, 5} | b = {4, 11}
  // TEST_CHECK(t[2].a.getKey() == r[4].getKey());
  //  TEST_CHECK(t[2].b.getKey() == s[1].getKey());

  TEST_CHECK(t[2].rowid_1 == 5);
  TEST_CHECK(t[2].rowid_2 == 11);
}

/* Test Join without Partitioning
 * ------------------------------
 */
void test_join_3() {
  tuple* tuples1 = new tuple[3]{{5, 1}, {2, 2}, {6, 3}};
  tuple* tuples2 = new tuple[5]{{2, 6}, {6, 7}, {1, 8}, {5, 10}, {20, 11}};

  relation r(tuples1, 3);
  relation s(tuples2, 5);
  result t = PartitionedHashJoin_ST(r, s, 0, 4, 8);

  TEST_CHECK(t.getSize() == 3);

  // a = {2, 2} | b = {2, 6}
  // TEST_CHECK(t[0].a.getKey() == r[1].getKey());
  // TEST_CHECK(t[0].b.getKey() == s[0].getKey());

  TEST_CHECK(t[0].rowid_1 == 2);
  TEST_CHECK(t[0].rowid_2 == 6);

  // a = {6, 3} | b = {6, 7}
  // TEST_CHECK(t[1].a.getKey() == r[2].getKey());
  // TEST_CHECK(t[1].b.getKey() == s[1].getKey());

  TEST_CHECK(t[1].rowid_1 == 3);
  TEST_CHECK(t[1].rowid_2 == 7);

  // a = {5, 1} | b = {5, 10}
  // TEST_CHECK(t[2].a.getKey() == r[0].getKey());
  // TEST_CHECK(t[2].b.getKey() == s[3].getKey());

  TEST_CHECK(t[2].rowid_1 == 1);
  TEST_CHECK(t[2].rowid_2 == 10);
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

  result t = PartitionedHashJoin_ST(r, s);
  TEST_CHECK(t.getSize() == 219);
}

/* Join Huge
 * ---------
 */
void test_join_5() {
  int64_t keys1[] = {
      669, 720, 897, 43,  226, 663, 879, 352, 222, 309, 526, 804, 63,  638, 717,
      547, 699, 755, 347, 972, 367, 973, 595, 440, 460, 381, 93,  344, 291, 369,
      185, 836, 986, 266, 734, 203, 929, 226, 752, 133, 11,  188, 845, 407, 44,
      401, 709, 471, 609, 769, 613, 371, 549, 798, 89,  32,  604, 829, 141, 647,
      264, 97,  683, 815, 334, 977, 744, 302, 919, 617, 932, 198, 19,  108, 117,
      353, 125, 812, 811, 161, 122, 78,  33,  188, 665, 738, 572, 761, 749, 858,
      373, 638, 653, 131, 298, 330, 336, 886, 784, 811, 445, 673, 153, 767, 496,
      242, 157, 280, 434, 576, 3,   673, 387, 988, 86,  706, 811, 19,  490, 838,
      30,  919, 165, 563, 526, 459, 518, 552, 250, 468, 599, 87,  219, 745, 924,
      784, 219, 844, 342, 191, 970, 95,  568, 279, 482, 148, 98,  184, 765, 878,
      17,  142, 732, 910, 995, 339, 677, 733, 871, 276, 231, 649, 676, 428, 290,
      273, 511, 627, 613, 127, 725, 752, 258, 255, 650, 523, 154, 227, 71,  312,
      96,  640, 794, 428, 573, 355, 170, 236, 251, 159, 718, 855, 589, 412, 387,
      609, 846, 614, 697, 12,  664, 991, 735, 455, 751, 515, 371, 441, 997, 518,
      402, 358, 69,  793, 639, 45,  459, 971, 716, 975, 343, 525, 939, 469, 279,
      53,  806, 283, 436, 4,   900, 348, 368, 922, 649, 726, 366, 977, 453, 564,
      355, 204, 665, 792, 565, 855, 42,  402, 352, 70,  433, 248, 905, 287, 60,
      312, 416, 583, 682, 779, 491, 935, 485, 74,  652, 12,  916, 205, 492, 14,
      437, 90,  621, 322, 812, 354, 762, 298, 358, 568, 667, 749, 844, 625, 438,
      444, 614, 134, 118, 556, 923, 992, 596, 998, 869, 826, 443, 607, 414, 438,
      160, 217, 471, 247, 254, 159, 153, 894, 694, 141, 191, 345, 577, 777, 842,
      272, 977, 738, 690, 173, 99,  735, 310, 319, 28,  218, 807, 449, 151, 760,
      684, 144, 83,  947, 981, 831, 471, 382, 563, 92,  185, 744, 379, 351, 487,
      511, 32,  614, 502, 235, 596, 813, 621, 502, 592, 311, 667, 37,  193, 361,
      210, 913, 193, 253, 125, 483, 363, 341, 349, 209, 328, 552, 550, 492, 387,
      616, 579, 294, 706, 374, 136, 350, 616, 549, 649, 371, 167, 274, 276, 346,
      624, 129, 292, 442, 709, 719, 789, 389, 348, 171, 780, 478, 244, 699, 4,
      410, 946, 759, 163, 207, 679, 76,  51,  470, 177, 641, 222, 16,  241, 179,
      860, 153, 427, 36,  457, 752, 99,  576, 118, 881, 290, 516, 861, 688, 379,
      245, 913, 928, 851, 593, 549, 657, 943, 755, 751, 939, 144, 956, 396, 803,
      679, 389, 726, 473, 378, 713, 563, 313, 284, 302, 445, 204, 405, 792, 789,
      839, 645, 952, 139, 927, 8,   431, 159, 269, 720, 623, 829, 872, 278, 603,
      591, 738, 53,  662, 542, 112, 147, 358, 533, 999, 810, 901, 441, 771, 651,
      821, 188, 963, 58,  543, 434, 446, 725, 736, 487, 164, 82,  613, 165, 915,
      831, 403, 207, 99,  505, 534, 619, 876, 532, 480, 374, 257, 481, 669, 928,
      410, 223, 68,  896, 231, 882, 539, 858, 244, 237, 354, 145, 273, 259, 701,
      542, 634, 363, 480, 606, 787, 855, 72,  883, 101, 93,  662, 807, 512, 192,
      7,   333, 933, 930, 389, 757, 798, 413, 877, 482, 393, 707, 483, 580, 105,
      374, 750, 925, 520, 978, 761, 349, 107, 770, 42,  994, 914, 682, 882, 807,
      75,  259, 688, 324, 211, 872, 535, 773, 259, 741, 359, 567, 382, 507, 445,
      389, 263, 639, 429, 20,  335, 675, 926, 957, 345, 947, 230, 720, 334, 717,
      739, 195, 190, 247, 988, 949, 712, 300, 329, 62,  590, 959, 13,  867, 559,
      279, 118, 523, 176, 867, 866, 365, 610, 231, 249, 678, 739, 967, 549, 276,
      736, 658, 116, 976, 766, 750, 529, 997, 801, 778, 803, 511, 252, 903, 49,
      466, 725, 269, 76,  349, 6,   414, 291, 161, 599, 943, 825, 686, 471, 64,
      492, 51,  430, 746, 741, 482, 314, 648, 146, 634, 583, 718, 96,  729, 711,
      608, 389, 349, 372, 519, 499, 490, 202, 944, 307, 23,  217, 657, 659, 332,
      753, 165, 681, 605, 248, 559, 602, 247, 891, 976, 804, 277, 23,  436, 300,
      111, 303, 402, 540, 864, 609, 629, 820, 436, 590, 980, 161, 645, 266, 417,
      310, 557, 782, 744, 95,  654, 109, 259, 140, 282, 531, 622, 940, 750, 208,
      711, 228, 449, 629, 126, 140, 680, 357, 212, 258, 574, 188, 891, 28,  647,
      379, 818, 924, 489, 821, 411, 168, 337, 206, 401, 909, 174, 400, 851, 187,
      442, 987, 554, 18,  663, 976, 508, 463, 473, 477, 928, 74,  612, 872, 962,
      424, 430, 238, 365, 874, 31,  55,  455, 652, 521, 132, 155, 161, 93,  141,
      795, 157, 381, 952, 196, 147, 266, 785, 833, 159, 483, 87,  596, 61,  757,
      186, 130, 515, 893, 193, 142, 549, 389, 905, 199, 375, 62,  979, 741, 185,
      533, 224, 829, 350, 981, 655, 878, 422, 528, 29,  39,  19,  217, 518, 745,
      516, 416, 153, 444, 782, 559, 50,  535, 124, 926, 489, 107, 374, 591, 288,
      555, 956, 908, 297, 464, 924, 693, 454, 402, 972, 981, 266, 302, 882, 334,
      760, 790, 263, 81,  109, 808, 370, 579, 34,  99,  949, 675, 924, 602, 8,
      163, 780, 551, 844, 339, 929, 610, 536, 825, 41,  791, 37,  656, 51,  280,
      30,  723, 271, 630, 925, 230, 408, 382, 929, 820, 465, 640, 826, 723, 163,
      157, 245, 56,  203, 697, 646, 919, 691, 680, 738, 438, 392, 698, 585, 209,
      293, 940, 975, 793, 706, 694, 257, 896, 530, 942, 606, 69,  492, 416, 916,
      555, 455, 344, 100, 364, 924, 320, 712, 46,  818, 712, 826, 809, 144, 658,
      528, 764, 506, 941, 882, 543, 381, 183, 396, 820, 224, 173, 256, 412, 866,
      186, 428, 925, 706, 283, 505, 916, 238, 903, 587, 439, 953, 979, 450, 57,
      316, 126, 110, 459, 397, 170, 308, 547, 3,   209, 903, 290, 867, 41,  919,
      814, 518, 896, 667, 84,  27,  11,  753, 817, 183, 172, 808, 569, 484, 582,
      806, 784, 801, 708, 32,  78,  759, 734, 220, 838, 515, 207, 359, 786, 335,
      706, 921, 679, 282, 255, 499, 246, 901, 936, 996, 889, 653, 367, 547, 754,
      699, 347, 483, 619, 927, 755, 909, 824, 878, 795, 872, 187, 627, 895, 542,
      668, 546, 899, 995, 10,  503, 607, 483, 470, 930, 188, 104, 265, 85,  71,
      587, 332, 405, 379, 462, 78,  234, 989, 254, 594, 54,  610, 187, 480, 960,
      45,  614, 969, 997, 564, 124, 686, 711, 669, 747, 217, 600, 420, 108, 321,
      477, 11,  458, 482, 11,  186, 150, 862, 562, 46,  479, 953, 656, 620, 347,
      539, 160, 240, 569, 608, 565, 715, 387, 975, 773, 440, 91,  299, 386, 791,
      404, 252, 551, 587, 545, 2,   63,  373, 8,   643, 121, 54,  721, 563, 155,
      888, 66,  643, 773, 532, 457, 496, 153, 788, 432, 471, 88,  138, 354, 615,
      424, 503, 178, 215, 826, 973, 582, 34,  625, 203, 233, 404, 12,  229, 740,
      781, 78,  413, 490, 820, 334, 964, 946, 357, 971, 940, 69,  886, 592, 783,
      839, 136, 489, 414, 170, 928, 553, 380, 532, 242, 728, 435, 423, 25,  155,
      938, 77,  886, 606, 92,  318, 343, 804, 941, 84,  197, 699, 543, 783, 193,
      95,  699, 511, 789, 336, 284, 889, 644, 48,  377, 258, 237, 779, 110, 668,
      136, 603, 102, 746, 123, 528, 141, 641, 193, 135, 878, 203, 671, 37,  561,
      684, 763, 781, 743, 284, 160, 121, 934, 56,  720, 240, 892, 69,  521, 132,
      41,  213, 541, 672, 417, 258, 949, 449, 502, 668, 80,  987, 912, 368, 88,
      914, 106, 829, 149, 196, 172, 524, 588, 313, 573, 324, 329, 338, 472, 752,
      440, 824, 187, 478, 974, 410, 913, 880, 308, 859, 440, 930, 420, 460, 60,
      865, 971, 250, 155, 713, 717, 764, 634, 652, 864, 551, 159, 923, 63,  839,
      428, 37,  463, 915, 282, 279, 502, 282, 604, 34,  101, 588, 352, 231, 462,
      98,  115, 193, 495, 319, 772, 78,  974, 396, 29,  403, 782, 266, 592, 995,
      990, 478, 188, 170, 539, 241, 375, 641, 61,  831, 582, 561, 46,  879, 260,
      184, 427, 990, 932, 355, 62,  136, 256, 55,  800, 191, 385, 378, 490, 23,
      994, 48,  906, 218, 626, 815, 968, 795, 267, 980, 707, 784, 47,  638, 893,
      462, 18,  421, 512, 85,  642, 396, 194, 614, 136, 547, 189, 960, 329, 206,
      792, 547, 553, 136, 621, 485, 400, 482, 309, 486, 191, 179, 955, 255, 110,
      797, 102, 178, 103, 231, 533, 847, 442, 979, 308, 551, 200, 303, 352, 845,
      4,   264, 635, 537, 667, 791, 307, 50,  713, 806, 830, 366, 295, 618, 705,
      431, 240, 340, 284, 936, 547, 462, 189, 149, 389, 24,  926, 587, 135, 848,
      464, 282, 44,  645, 698, 313, 633, 693, 671, 997, 245, 82,  876, 427, 728,
      339, 256, 987, 27,  935, 471, 914, 639, 193, 291, 601, 513, 461, 164, 894,
      122, 725, 624, 892, 250, 728, 836, 803, 600, 741, 745, 587, 883, 734, 401,
      505, 707, 436, 822, 54,  791, 620, 503, 503, 987, 247, 954, 572, 563, 162,
      469, 124, 243, 182, 658, 296, 327, 511, 860, 103, 788, 484, 237, 363, 591,
      817, 817, 303, 917, 30,  660, 174, 919, 531, 472, 323, 505, 617, 69,  593,
      201, 123, 344, 173, 634, 84,  156, 772, 253, 20,  606, 78,  240, 592, 463,
      14,  98,  514, 958, 246, 9,   624, 383, 565, 441, 569, 971, 979, 243, 620,
      24,  40,  661, 576, 987, 4,   599, 486, 126, 240, 742, 570, 173, 21,  609,
      482, 485, 459, 383, 830, 485, 238, 442, 231, 994, 550, 365, 437, 176, 728,
      28,  874, 797, 204, 100, 278, 341, 329, 389, 773, 378, 230, 338, 941, 130,
      891, 709, 669, 436, 607, 239, 121, 812, 687, 664, 46,  424, 381, 205, 45,
      577, 233, 268, 945, 89,  728, 289, 484, 900, 899, 535, 689, 700, 144, 363,
      178, 559, 657, 206, 743, 630, 517, 938, 917, 840, 127, 68,  301, 939, 72,
      881, 417, 843, 269, 776, 454, 267, 512, 455, 775, 856, 312, 641, 316, 490,
      721, 897, 295, 211, 76,  929, 348, 306, 171, 28,  632, 441, 811, 917, 797,
      2,   327, 703, 961, 470, 441, 452, 856, 525, 454, 692, 613, 367, 993, 703,
      408, 757, 868, 173, 559, 484, 139, 348, 26,  91,  619, 365, 413, 2,   161,
      912, 709, 46,  13,  207, 295, 192, 859, 658, 546, 112, 802, 890, 368, 358,
      656, 317, 355, 964, 310, 136, 792, 782, 158, 719, 452, 989, 49,  52,  680,
      831, 534, 583, 929, 414, 881, 277, 367, 322, 805, 263, 944, 272, 296, 424,
      436, 895, 382, 464, 518, 856, 921, 98,  461, 689, 354, 335, 891, 990, 577,
      523, 670, 181, 290, 753, 336, 611, 622, 165, 454, 840, 253, 223, 297, 894,
      199, 868, 347, 327, 24,  384, 875, 168, 103, 355, 353, 118, 908, 96,  86,
      785, 599, 705, 463, 51,  862, 80,  797, 359, 311, 472, 318, 26,  574, 897,
      179, 870, 110, 501, 435, 76,  636, 915, 305, 480, 455, 656, 582, 365, 801,
      861, 358, 360, 276, 151, 317, 50,  661, 378, 32,  328, 564, 138, 207, 423,
      115, 139, 878, 719, 469, 803, 436, 117, 860, 754, 257, 168, 983, 80,  851,
      874, 975, 43,  206, 635, 829, 635, 931, 558, 139, 19,  775, 901, 714, 553,
      102, 378, 826, 373, 27,  924, 643, 765, 654, 775, 932, 606, 500, 192, 467,
      253, 354, 518, 239, 97,  164, 11,  516, 342, 570, 516, 131, 485, 996, 896,
      868, 106, 491, 301, 836, 996, 671, 448, 367, 152, 732, 756, 523, 109, 941,
      634, 494, 989, 982, 473, 619, 798, 635, 364, 462, 664, 989, 168, 508, 824,
      928, 638, 544, 599, 205, 969, 986, 276, 712, 82,  320, 264, 99,  677, 118,
      933, 421, 153, 906, 770, 249, 561, 319, 178, 569, 825, 243, 618, 555, 599,
      682, 20,  662, 482, 719, 144, 247, 717, 783, 189, 713, 254, 536, 313, 435,
      667, 745, 874, 984, 484, 263, 119, 334, 367, 130, 868, 401, 283, 916, 463,
      20,  867, 154, 396, 544, 665, 428, 275, 640, 709, 897, 970, 327, 839, 799,
      646, 27,  529, 740, 788, 360, 756, 167, 735, 104, 373, 253, 650, 816, 490,
      58,  681, 939, 273, 144, 772, 670, 698, 425, 303, 485, 265, 113, 404, 837,
      544, 418, 530, 674, 678, 696, 845, 623, 773, 410, 724, 904, 329, 186, 306,
      826, 620, 815, 570, 178, 578, 249, 794, 82,  462, 999, 116, 630, 190, 124,
      176, 983, 520, 52,  574, 857, 476, 392, 724, 376, 653, 895, 776, 90,  165,
      871, 90,  934, 924, 394, 172, 984, 312, 594, 627, 846, 5,   377, 910, 520,
      329, 892, 170, 952, 527, 18,  475, 963, 165, 240, 528, 58,  369, 50,  71,
      575, 680, 769, 233, 708, 993, 724, 124, 531, 523, 196, 490, 881, 23,  309,
      78,  987, 801, 878, 617, 733, 630, 830, 809, 310, 799, 162, 380, 919, 716,
      825, 291, 171, 859, 581, 144, 540, 6,   234, 563, 637, 22,  290, 927, 35,
      380, 228, 664, 637, 636, 84,  776, 289, 783, 37,  782, 167, 464, 95,  275,
      629, 621, 685, 709, 353, 8,   838, 594, 477, 56,  934, 906, 341, 955, 917,
      605, 891, 898, 814, 983, 194, 376, 831, 197, 814, 7,   638, 629, 885, 213,
      610, 503, 50,  708, 587, 114, 242, 900, 973, 976, 271, 688, 422, 750, 393,
      10,  919, 564, 226, 708, 612, 348, 790, 488, 541, 563, 82,  870, 802, 340,
      440, 883, 383, 861, 236, 8,   181, 171, 157, 110, 960, 542, 833, 87,  665,
      46,  578, 656, 421, 325, 380, 724, 20,  465, 112, 78,  491, 580, 853, 464,
      912, 933, 468, 225, 273, 541, 71,  405, 663, 296, 203, 81,  472, 32,  462,
      289, 599, 654, 778, 572, 743, 382, 813, 681, 225, 813, 707, 466, 954, 815,
      535, 867, 301, 312, 150, 128, 731, 619, 687, 899, 454, 52,  893, 149, 908,
      642, 244, 100, 604, 257, 206, 724, 988, 138, 383, 95,  182, 856, 511, 918,
      498, 225, 69,  83,  664, 880, 758, 722, 892, 442, 223, 670, 766, 110, 328,
      858, 954, 272, 44,  638, 874, 296, 189, 460, 105, 254, 71,  240, 263, 500,
      238, 696, 564, 379, 683, 445, 804, 701, 270, 67,  708, 645, 189, 815, 653,
      231, 560, 997, 492, 367, 137, 222, 275, 671, 128, 699, 922, 613, 241, 411,
      734, 317, 112, 522, 396, 401, 213, 30,  392, 779, 95,  492, 26,  553, 548,
      459, 974, 131, 42,  379, 589, 158, 190, 674, 390, 544, 214, 471, 602, 218,
      124, 438, 109, 475, 623, 10,  698, 947, 462, 630, 936, 452, 972, 224, 29,
      359, 626, 16,  722, 434, 498, 826, 526, 336, 731, 664, 992, 316, 399, 45,
      440, 191, 366, 517, 136, 287, 395, 734, 496, 994, 972, 837, 949, 211, 713,
      71,  9,   385, 247, 655, 973, 959, 219, 84,  94,  958, 614, 389, 536, 105,
      6,   347, 121, 30,  345, 834, 248, 879, 18,  992, 165, 399, 785, 704, 133,
      256, 992, 322, 686, 51,  207, 899, 590, 240, 715, 663, 113, 984, 618, 18,
      549, 360, 960, 514, 629, 590, 971, 173, 870, 418, 205, 405, 64,  967, 847,
      743, 419, 452, 394, 165, 967, 704, 694, 524, 236, 93,  217, 530, 772, 439,
      102, 717, 877, 303, 2,   609, 19,  830, 263, 782, 472, 333, 272, 626, 119,
      188, 787, 615, 498, 537, 682, 928, 380, 538, 608, 404, 970, 594, 186, 699,
      452, 481, 901, 496, 183, 714, 878, 793, 120, 486, 285, 461, 599, 806, 713,
      737, 863, 643, 371, 578, 86,  202, 687, 449, 783, 701, 170, 628, 246, 217,
      710, 478, 286, 676, 541, 440, 645, 692, 389, 413, 639, 503, 184, 645, 761,
      69,  773, 172, 884, 327, 881, 892, 797, 60,  496, 347, 784, 248, 384, 394,
      162, 980, 425, 157, 283, 693, 494, 187, 20,  597, 930, 785, 49,  627, 263,
      942, 10,  734, 178, 718, 780, 291, 771, 259, 923, 693, 723, 719, 533, 884,
      212, 35,  245, 370, 572, 881, 604, 261, 537, 303, 998, 83,  132, 868, 755,
      522, 390, 181, 528, 114, 961, 844, 264, 200, 918, 53,  862, 248, 829, 267,
      843, 968, 697, 20,  206, 450, 495, 528, 907, 86,  387, 2,   916, 637, 291,
      963, 272, 88,  87,  957, 716, 832, 168, 419, 272, 851, 884, 5,   761, 848,
      452, 533, 970, 993, 441, 641, 750, 561, 875, 170, 975, 146, 299, 638, 934,
      812, 798, 89,  75,  227, 858, 595, 822, 774, 528, 185, 939, 397, 549, 985,
      589, 479, 921, 201, 135, 571, 593, 795, 318, 377, 632, 791, 280, 260, 477,
      975, 164, 261, 203, 544, 120, 888, 73,  800, 134, 992, 990, 936, 216, 380,
      357, 109, 983, 109, 992, 468, 308, 319, 712, 444, 970, 312, 595, 293, 523,
      638, 713, 569, 135, 941, 908, 746, 714, 190, 292, 576, 935, 619, 471, 862,
      385, 864, 952, 839, 912, 602, 488, 171, 803, 42,  541, 380, 715, 609, 881,
      838, 43,  571, 541, 263, 765, 511, 366, 862, 690, 705, 727, 342, 961, 975,
      773, 316, 482, 483, 244, 175, 335, 997, 394, 821, 265, 598, 660, 270, 793,
      693, 166, 269, 709, 301, 854, 303, 403, 592, 876, 371, 173, 426, 722, 789,
      813, 482, 4,   252, 917, 140, 783, 366, 729, 562, 432, 743, 29,  919, 686,
      398, 441, 536, 585, 730, 770, 361, 880, 415, 932, 211, 693, 428, 363, 245,
      715, 734, 121, 182, 63,  340, 590, 49,  789, 104, 730, 105, 380, 574, 360,
      90,  721, 229, 486, 93,  807, 388, 351, 935, 281, 65,  654, 147, 142, 580,
      749, 518, 215, 84,  3,   929, 874, 629, 278, 660, 817, 38,  247, 713, 405,
      415, 781, 918, 68,  905, 204, 894, 493, 139, 745, 485, 486, 865, 237, 543,
      303, 670, 833, 448, 972, 910, 721, 19,  315, 810, 791, 767, 282, 196, 558,
      989, 537, 471, 697, 828, 275, 760, 968, 589, 498, 816, 416, 255, 887, 925,
      750, 400, 819, 587, 877, 70,  183, 774, 37,  283, 82,  489, 52,  736, 23,
      885, 510, 574, 506, 937, 843, 299, 662, 890, 921, 593, 173, 849, 795, 793,
      599, 175, 366, 567, 930, 415, 923, 633, 892, 608, 220, 947, 52,  295, 295,
      153, 488, 360, 293, 7,   616, 496, 414, 395, 42,  530, 563, 619, 349, 235,
      52,  934, 115, 400, 359, 717, 770, 995, 946, 911, 702, 48,  436, 695, 111,
      810, 30,  507, 495, 434, 458, 473, 713, 926, 629, 7,   508, 994, 935, 503,
      253, 369, 738, 266, 95,  895, 768, 474, 201, 189, 5,   328, 586, 727, 264,
      194, 615, 411, 385, 817, 898, 945, 250, 556, 292, 970, 145, 151, 806, 64,
      353, 763, 495, 758, 300, 765, 832, 523, 337, 331, 254, 103, 46,  719, 393,
      349, 414, 855, 420, 407, 463, 314, 31,  999, 397, 975, 223, 886, 689, 255,
      900, 129, 631, 237, 748, 648, 821, 512, 220, 91,  22,  937, 696, 261, 332,
      119, 208, 833, 570, 101, 471, 690, 93,  510, 6,   69,  270, 245, 604, 233,
      928, 759, 805, 299, 491, 20,  292, 94,  250, 810, 121, 802, 558, 262, 469,
      965, 197, 51,  215, 790, 521, 427, 732, 534, 750, 487, 265, 45,  618, 413,
      818, 80,  178, 743, 937, 645, 263, 736, 199, 927, 432, 962, 20,  917, 398,
      869, 634, 54,  230, 583, 26,  880, 218, 70,  131, 120, 975, 32,  427, 673,
      593, 989, 230, 273, 195, 787, 9,   618, 160, 126, 640, 368, 848, 522, 252,
      697, 303, 834, 668, 498, 194, 173, 256, 133, 46,  992, 961, 630, 867, 872,
      816, 40,  412, 587, 813, 815, 937, 666, 289, 406, 602, 579, 102, 612, 480,
      542, 602, 169, 363, 536, 554, 664, 623, 306, 457, 568, 559, 547, 255, 298,
      380, 811, 802, 279, 887, 322, 862, 550, 98,  490, 484, 636, 497, 419, 700,
      409, 751, 724, 656, 188, 159, 896, 640, 863, 867, 373, 830, 523, 793, 412,
      163, 704, 843, 636, 643, 824, 347, 888, 470, 201, 930, 267, 259, 202, 336,
      834, 59,  899, 362, 452, 995, 201, 248, 318, 779, 754, 254, 59,  547, 678,
      55,  125, 710, 463, 387, 409, 679, 199, 967, 672, 87,  636, 208, 739, 23,
      788, 641, 113, 66,  512, 111, 501, 590, 942, 20,  503, 878, 289, 225, 775,
      555, 965, 394, 627, 291, 566, 821, 541, 143, 736, 115, 303, 164, 801, 614,
      161, 22,  865, 627, 914, 397, 422, 550, 619, 713, 830, 842, 423, 144, 295,
      942, 205, 260, 807, 139, 706, 884, 204, 195, 136, 763, 211, 491, 708, 417,
      611, 978, 846, 458, 363, 782, 206, 674, 646, 848, 559, 977, 349, 772, 221,
      421, 685, 547, 155, 32,  457, 179, 402, 217, 13,  130, 268, 434, 137, 731,
      927, 532, 667, 745, 860, 912, 855, 240, 563, 101, 583, 873, 937, 77,  743,
      959, 482, 392, 472, 482, 579, 442, 587, 894, 173, 439, 655, 689, 691, 979,
      521, 988, 559, 508, 623, 296, 963, 832, 926, 768, 729, 370, 75,  896, 331,
      656, 372, 850, 219, 348, 188, 499, 895, 217, 328, 681, 262, 252, 745, 53,
      816, 899, 290, 674, 783, 750, 895, 226, 173, 813, 657, 307, 449, 8,   339,
      901, 263, 119, 384, 987, 424, 342, 128, 633, 354, 896, 522, 824, 854, 664,
      538, 73,  632, 510, 978, 871, 842, 244, 190, 925, 399, 357, 230, 843, 184,
      163, 456, 995, 50,  102, 965, 133, 982, 902, 128, 881, 823, 91,  116, 453,
      591, 501, 767, 178, 833, 801, 43,  493, 381, 217, 510, 773, 328, 128, 350,
      294, 629, 874, 175, 141, 752, 714, 416, 895, 993, 78,  676, 127, 169, 282,
      394, 401, 843, 662, 251, 111, 810, 484, 437, 677, 877, 415, 420, 36,  215,
      772, 7,   477, 981, 552, 459, 101, 411, 301, 60,  171, 128, 302, 20,  506,
      451, 138, 985, 409, 144, 382, 626, 593, 761, 8,   531, 743, 70,  794, 196,
      407, 145, 386, 818, 35,  618, 694, 935, 803, 657, 723, 158, 490, 921, 314,
      398, 861, 148, 250, 576, 495, 696, 982, 605, 441, 373, 288, 977, 370, 666,
      270, 478, 6,   39,  348, 269, 855, 300, 32,  651, 307, 447, 478, 896, 41,
      632, 28,  87,  670, 101, 915, 953, 11,  475, 283, 713, 821, 869, 678, 148,
      941, 718, 73,  958, 643, 639, 542, 116, 90,  385, 908, 345, 823, 953, 787,
      3,   29,  156, 307, 501, 781, 969, 715, 195, 89,  74,  668, 225, 493, 699,
      167, 37,  162, 529, 521, 419, 253, 218, 27,  949, 934, 317, 576, 557, 254,
      769, 385, 329, 724, 668, 284, 548, 701, 543, 265, 200, 11,  910, 315, 146,
      557, 759, 376, 213, 601, 565, 281, 585, 645, 366, 789, 56,  964, 370, 760,
      284, 839, 310, 62,  892, 92,  244, 212, 733, 376, 903, 34,  815, 711, 545,
      646, 452, 210, 5,   665, 745, 895, 392, 125, 450, 318, 904, 158, 186, 725,
      204, 92,  475, 843, 166, 785, 641, 5,   120, 261, 66,  420, 887, 834, 685,
      332, 587, 641, 353, 227, 415, 472, 977, 123, 83,  959, 761, 345, 148, 581,
      787, 889, 528, 41,  198, 233, 918, 705, 306, 538, 679, 204, 287, 123, 702,
      881, 509, 876, 230, 333, 49,  282, 320, 129, 498, 93,  813, 862, 514, 636,
      825, 437, 950, 276, 419, 7,   365, 907, 538, 513, 295, 525, 718, 135, 83,
      964, 807, 462, 430, 756, 668, 455, 222, 899, 882, 686, 958, 120, 547, 404,
      717, 893, 918, 73,  261, 11,  626, 292, 421, 387, 170, 724, 649, 764, 738,
      220, 873, 379, 284, 570, 428, 590, 369, 289, 749, 642, 81,  341, 456, 14,
      600, 136, 501, 129, 383, 134, 333, 431, 591, 6,   798, 146, 751, 903, 261,
      389, 604, 27,  97,  297, 604, 139, 677, 699, 481, 262, 805, 976, 585, 306,
      833, 45,  976, 537, 389, 112, 103, 82,  406, 742, 12,  847, 82,  188, 71,
      233, 659, 566, 581, 828, 5,   80,  361, 261, 17,  599, 193, 157, 423, 934,
      405, 169, 989, 870, 400, 74,  599, 493, 813, 556, 557, 151, 476, 524, 913,
      114, 112, 95,  790, 54,  814, 991, 444, 132, 475, 206, 79,  266, 299, 102,
      921, 50,  192, 918, 418, 476, 845, 393, 707, 756, 307, 792, 810, 257, 52,
      406, 267, 91,  372, 987, 505, 607, 412, 447, 567, 527, 709, 585, 452, 696,
      191, 912, 815, 849, 828, 415, 528, 53,  807, 553, 970, 601, 876, 553, 570,
      896, 956, 355, 537, 504, 639, 984, 527, 839, 243, 11,  724, 488, 149, 751,
      458, 189, 240, 226, 120, 157, 884, 55,  317, 893, 247, 627, 236, 741, 816,
      775, 163, 629, 203, 949, 886, 973, 193, 478, 311, 631, 658, 565, 563, 369,
      961, 64,  457, 183, 401, 881, 295, 83,  10,  871, 414, 402, 480, 99,  874,
      816, 534, 234, 910, 280, 340, 381, 365, 720, 413, 442, 531, 547, 905, 361,
      855, 974, 594, 509, 168, 509, 309, 410, 759, 267, 391, 752, 15,  993, 924,
      980, 514, 731, 692, 31,  601, 499, 583, 104, 780, 33,  965, 291, 492, 316,
      398, 97,  51,  713, 282, 370, 76,  774, 747, 634, 935, 376, 89,  585, 384,
      735, 751, 469, 74,  956, 333, 215, 713, 481, 367, 899, 879, 667, 665, 391,
      27,  817, 618, 539, 678, 518, 425, 47,  324, 483, 169, 594, 483, 135, 293,
      981, 125, 981, 738, 563, 464, 820, 569, 368, 132, 632, 487, 627, 243, 150,
      756, 971, 147, 960, 547, 652, 943, 730, 258, 42,  498, 171, 896, 538, 192,
      429, 827, 983, 399, 504, 364, 835, 25,  681, 611, 328, 218, 915, 10,  949,
      289, 384, 685, 555, 464, 234, 363, 359, 406, 829, 593, 913, 365, 751, 713,
      694, 584, 446, 559, 719, 223, 196, 953, 465, 405, 416, 169, 580, 640, 65,
      61,  164, 163, 892, 28,  2,   918, 489, 108, 789, 710, 111, 219, 80,  488,
      108, 506, 317, 381, 776, 159, 71,  245, 3,   724, 747, 945, 964, 831, 590,
      815, 651, 816, 900, 333, 78,  943, 225, 422, 655, 159, 291, 45,  785, 301,
      903, 98,  570, 812, 650, 491, 232, 805, 818, 700, 774, 293, 277, 983, 416,
      279, 481, 400, 873, 411, 471, 344, 127, 11,  193, 209, 410, 852, 103, 982,
      456, 208, 793, 962, 556, 367, 36,  483, 352, 257, 361, 953, 167, 612, 832,
      748, 172, 673, 259, 462, 739, 648, 742, 908, 552, 979, 564, 633, 836, 280,
      879, 782, 824, 629, 133, 350, 882, 555, 357, 105, 365, 561, 456, 401, 226,
      29,  872, 310, 857, 288, 720, 358, 729, 632, 54,  660, 615, 502, 949, 730,
      167, 703, 515, 483, 225, 520, 153, 229, 956, 14,  599, 45,  516, 845, 408,
      153, 564, 411, 963, 828, 220, 600, 508, 58,  900, 200, 574, 240, 315, 464,
      983, 847, 455, 880, 635, 846, 279, 650, 616, 613, 736, 855, 1,   748, 123,
      41,  314, 648, 529, 613, 621, 759, 193, 12,  530, 370, 937, 35,  727, 701,
      714, 336, 589, 332, 103, 321, 393, 419, 629, 24,  694, 926, 746, 554, 94,
      986, 12,  685, 589, 588, 558, 452, 536, 272, 473, 50,  459, 613, 41,  19,
      767, 166, 554, 510, 29,  288, 516, 610, 114, 385, 519, 800, 582, 685, 872,
      722, 777, 609, 492, 861, 54,  178, 345, 266, 303, 197, 292, 746, 891, 311,
      945, 901, 466, 109, 404, 304, 240, 955, 197, 640, 868, 384, 943, 911, 830,
      233, 147, 532, 490, 309, 468, 858, 699, 933, 473, 714, 509, 523, 161, 793,
      260, 590, 608, 58,  762, 124, 755, 683, 948, 207, 853, 870, 404, 134, 533,
      528, 221, 544, 884, 260, 655, 524, 535, 734, 604, 56,  210, 215, 954, 290,
      172, 401, 329, 146, 95,  455, 129, 572, 982, 400, 581, 6,   658, 36,  65,
      514, 229, 595, 799, 887, 564, 681, 493, 904, 908, 953, 531, 493, 302, 953,
      18,  48,  32,  963, 733, 608, 359, 148, 554, 637, 924, 670, 954, 945, 498,
      29,  100, 554, 898, 655, 373, 274, 338, 407, 21,  794, 140, 858, 26,  786,
      802, 546, 126, 667, 203, 224, 239, 12,  224, 348, 296, 500, 816, 127, 72,
      469, 982, 41,  49,  96,  12,  573, 372, 60,  316, 422, 314, 947, 739, 491,
      941, 357, 652, 260, 373, 971, 774, 663, 769, 631, 728, 144, 470, 75,  871,
      17,  625, 559, 51,  213, 874, 195, 101, 563, 77,  653, 558, 426, 719, 392,
      474, 240, 625, 849, 8,   360, 430, 595, 195, 545, 87,  961, 956, 717, 730,
      359, 546, 325, 15,  519, 534, 768, 49,  610, 959, 792, 555, 689, 54,  983,
      699, 505, 288, 813, 791, 994, 964, 651, 255, 311, 816, 509, 963, 189, 87,
      358, 958, 907, 638, 918, 180, 835, 220, 559, 744, 990, 573, 381, 917, 490,
      934, 648, 656, 300, 96,  262, 337, 430, 131, 880, 104, 818, 54,  92,  155,
      227, 556, 735, 927, 10,  656, 474, 897, 705, 659, 794, 811, 468, 75,  486,
      361, 656, 911, 957, 274, 206, 390, 677, 752, 920, 609, 336, 441, 320, 536,
      482, 72,  703, 822, 997, 607, 486, 478, 232, 812, 53,  884, 741, 441, 38,
      35,  707, 852, 272, 204, 61,  538, 7,   64,  279, 439, 280, 685, 875, 155,
      426, 565, 568, 761, 664, 965, 836, 715, 665, 35,  31,  92,  363, 736, 987,
      14,  118, 9,   209, 937, 479, 155, 839, 394, 120, 8,   85,  5,   788, 116,
      827, 185, 798, 582, 145, 675, 766, 385, 149, 570, 675, 176, 731, 142, 197,
      506, 495, 750, 154, 267, 392, 430, 688, 580, 953, 902, 290, 592, 508, 404,
      632, 539, 347, 663, 368, 890, 849, 778, 423, 98,  307, 6,   387, 710, 698,
      714, 629, 577, 580, 930, 711, 959, 946, 201, 475, 808, 215, 752, 244, 447,
      604, 537, 797, 11,  542, 734, 345, 76,  453, 381, 352, 972, 684, 445, 656,
      908, 865, 799, 728, 508, 291, 700, 740, 782, 63,  194, 416, 537, 906, 221,
      731, 988, 785, 588, 522, 761, 205, 143, 197, 479, 110, 801, 371, 677, 785,
      643, 573, 351, 194, 385, 457, 911, 722, 919, 552, 7,   524, 500, 400, 816,
      946, 919, 714, 678, 384, 965, 792, 814, 619, 198, 927, 341, 138, 53,  443,
      837, 422, 620, 589, 35,  654, 891, 409, 909, 256, 37,  435, 922, 313, 760,
      477, 650, 599, 316, 566, 458, 311, 31,  496, 141, 102, 222, 382, 834, 114,
      331, 508, 659, 941, 850, 243, 769, 206, 808, 796, 537, 381, 448, 585, 700,
      492, 961, 847, 878, 337, 560, 958, 290, 673, 319, 268, 895, 630, 984, 109,
      759, 817, 661, 437, 195, 453, 64,  664, 581, 411, 138, 781, 750, 986, 25,
      814, 500, 980, 578, 833, 258, 977, 855, 929, 337, 135, 285, 972, 152, 513,
      621, 40,  366, 654, 873, 959, 277, 407, 116, 692, 811, 900, 195, 398, 271,
      37,  574, 265, 328, 281, 676, 199, 149, 139, 301, 845, 207, 666, 117, 623,
      796, 463, 631, 788, 597, 138, 817, 230, 22,  462, 645, 159, 856, 189, 1,
      848, 900, 736, 691, 654, 656, 600, 695, 497, 400, 673, 195, 794, 935, 935,
      168, 572, 723, 226, 216, 767, 119, 945, 778, 427, 988, 406, 666, 727, 575,
      468, 864, 87,  755, 894, 258, 381, 732, 642, 323, 75,  59,  686, 3,   455,
      844, 292, 983, 740, 334, 497, 727, 917, 659, 650, 465, 535, 368, 548, 100,
      405, 713, 263, 939, 224, 31,  482, 127, 670, 519, 373, 164, 597, 609, 719,
      993, 662, 671, 572, 42,  261, 131, 941, 636, 258, 991, 516, 117, 665, 839,
      458, 81,  973, 255, 405, 658, 578, 809, 991, 551, 980, 116, 137, 260, 656,
      274, 59,  921, 589, 862, 330, 908, 956, 424, 555, 578, 484, 401, 766, 813,
      314, 316, 691, 509, 320, 501, 462, 815, 587, 239, 369, 461, 42,  42,  666,
      465, 182, 700, 272, 207, 222, 469, 380, 459, 221, 190, 536, 275, 297, 190,
      297, 119, 627, 446, 735, 336, 385, 142, 372, 283, 223, 203, 950, 360, 390,
      457, 752, 136, 767, 216, 201, 8,   767, 165, 864, 803, 422, 293, 821, 836,
      172, 188, 470, 464, 776, 285, 560, 576, 111, 521, 453, 749, 195, 750, 981,
      601, 134, 55,  165, 145, 361, 635, 807, 191, 744, 152, 425, 748, 317, 857,
      829, 317, 583, 499, 47,  11,  163, 195, 719, 55,  601, 982, 500, 82,  509,
      917, 352, 996, 609, 488, 235, 161, 843, 4,   18,  206, 5,   668, 784, 26,
      101, 854, 487, 297, 799, 774, 650, 504, 759, 615, 758, 425, 809, 683, 363,
      594, 806, 491, 977, 673, 952, 592, 123, 144, 59,  637, 52,  404, 576, 739,
      755, 569, 921, 401, 143, 899, 276, 203, 692, 872, 429, 142, 303, 802, 897,
      579, 636, 140, 269, 501, 17,  368, 51,  107, 481, 580, 140, 425, 602, 907,
      578, 912, 450, 443, 144, 255, 8,   441, 281, 369, 108, 801, 994, 652, 182,
      266, 95,  737, 266, 183, 403, 375, 268, 618, 698, 907, 869, 611, 581, 988,
      195, 704, 220, 126, 574, 92,  746, 260, 896, 755, 67,  827, 750, 472, 915,
      628, 276, 353, 327, 212, 992, 815, 897, 661, 100, 413, 490, 478, 616, 598,
      848, 9,   102, 419, 313, 324, 982, 532, 594, 456, 219, 102, 492, 615, 797,
      288, 182, 507, 75,  30,  888, 712, 876, 953, 358, 111, 675, 848, 89,  967,
      992, 306, 746, 609, 286, 922, 155, 248, 105, 502, 105, 398, 317, 412, 128,
      115, 887, 801, 856, 496, 666, 749, 563, 797, 275, 776, 514, 461, 581, 996,
      298, 579, 705, 115, 116, 219, 63,  490, 111, 83,  164, 378, 938, 472, 929,
      586, 169, 236, 426, 129, 801, 964, 309, 260, 517, 773, 730, 280, 292, 197,
      863, 672, 299, 876, 626, 750, 639, 60,  850, 730, 564, 198, 541, 127, 241,
      495, 502, 802, 514, 797, 416, 271, 663, 772, 145, 650, 649, 462, 118, 111,
      72,  480, 816, 625, 416, 964, 685, 929, 448, 460, 92,  2,   531, 912, 282,
      544, 608, 693, 468, 137, 473, 271, 415, 427, 889, 608, 233, 644, 178, 91,
      492, 556, 312, 479, 231, 776, 179, 35,  427, 587, 38,  695, 848, 93,  838,
      899, 920, 346, 84,  373, 4,   836, 11,  645, 786, 307, 526, 935, 926, 513,
      580, 311, 15,  757, 781, 322, 753, 179, 744, 694, 657, 938, 34,  923, 770,
      934, 682, 626, 437, 455, 933, 465, 229, 705, 71,  215, 807, 944, 343, 548,
      670, 707, 464, 733, 826, 625, 10,  876, 928, 305, 686, 186, 588, 359, 949,
      613, 154, 938, 957, 362, 96,  722, 746, 829, 42,  852, 956, 657, 801, 690,
      443, 916, 494, 546, 887, 61,  248, 234, 309, 20,  74,  693, 672, 727, 151,
      221, 42,  780, 487, 272, 495, 424, 277, 618, 591, 461, 696, 193, 250, 52,
      657, 465, 70,  970, 621, 740, 8,   436, 563, 939, 164, 463, 329, 408, 783,
      534, 453, 225, 115, 322, 859, 350, 370, 478, 201, 209, 168, 610, 23,  70,
      251, 161, 106, 777, 494, 274, 613, 733, 895, 92,  719, 587, 151, 862, 360,
      363, 393, 478, 445, 707, 831, 187, 675, 41,  144, 864, 186, 360, 442, 310,
      434, 805, 270, 693, 222, 120, 391, 638, 63,  814, 394, 781, 908, 451, 566,
      97,  850, 87,  881, 181, 481, 359, 19,  731, 489, 430, 574, 430, 48,  134,
      990, 475, 85,  148, 412, 831, 32,  67,  296, 720, 41,  721, 121, 996, 22,
      474, 229, 698, 60,  734, 76,  6,   791, 923, 897, 890, 166, 829, 272, 264,
      636, 562, 699, 374, 720, 866, 925, 727, 811, 628, 805, 507, 506, 908, 697,
      560, 889, 450, 922, 740, 979, 812, 775, 888, 678, 563, 15,  388, 482, 315,
      66,  311, 748, 171, 57,  21,  601, 440, 421, 221, 828, 145, 879, 262, 536,
      284, 241, 623, 224, 172, 758, 871, 808, 145, 511, 79,  318, 62,  31,  824,
      831, 826, 419, 925, 997, 522, 420, 519, 402, 411, 776, 39,  348, 171, 838,
      267, 481, 562, 579, 617, 674, 876, 233, 506, 860, 460, 928, 478, 836, 991,
      480, 505, 713, 717, 493, 623, 99,  560, 5,   450, 569, 661, 767, 768, 736,
      156, 256, 14,  290, 761, 599, 219, 762, 492, 516, 838, 423, 151, 338, 7,
      918, 25,  540, 483, 32,  727, 510, 475, 632, 398, 724, 293, 431, 82,  398,
      216, 260, 274, 342, 251, 193, 463, 157, 290, 301, 166, 411, 222, 351, 495,
      698, 868, 170, 38,  356, 177, 176, 736, 666, 639, 412, 454, 467, 818, 793,
      223, 43,  298, 745, 949, 810, 13,  445, 935, 610, 816, 769, 831, 947, 120,
      171, 855, 647, 928, 744, 895, 187, 995, 621, 314, 428, 839, 750, 724, 117,
      657, 949, 824, 342, 421, 754, 135, 237, 322, 534, 528, 846, 821, 37,  592,
      56,  260, 892, 243, 913, 272, 198, 21,  105, 88,  989, 196, 329, 660, 760,
      935, 934, 271, 500, 120, 159, 422, 476, 197, 471, 268, 697, 821, 286, 872,
      581, 321, 463, 282, 518, 802, 748, 485, 585, 245, 527, 737, 661, 71,  899,
      357, 786, 532, 783, 428, 695, 73,  397, 746, 491, 249, 752, 574, 690, 217,
      277, 281, 578, 245, 912, 969, 550, 732, 194, 274, 155, 931, 199, 116, 48,
      443, 380, 602, 837, 539, 96,  506, 6,   277, 948, 44,  380, 939, 6,   73,
      525, 887, 697, 13,  956, 217, 53,  943, 508, 3,   394, 702, 889, 465, 584,
      853, 352, 163, 53,  258, 621, 382, 265, 662, 897, 18,  944, 115, 699, 839,
      469, 962, 985, 461, 213, 187, 773, 399, 786, 239, 278, 501, 765, 253, 155,
      410, 867, 954, 661, 436, 39,  246, 154, 928, 360, 952, 32,  766, 706, 160,
      708, 923, 554, 467, 198, 811, 804, 731, 265, 968, 818, 103, 439, 961, 682,
      640, 356, 498, 755, 345, 611, 897, 648, 410, 465, 609, 772, 376, 448, 253,
      903, 753, 521, 386, 899, 310, 793, 566, 365, 86,  302, 921, 719, 685, 912,
      766, 639, 468, 640, 959, 120, 726, 648, 121, 537, 296, 383, 521, 680, 677,
      421, 545, 633, 208, 319, 958, 173, 437, 711, 570, 226, 38,  736, 42,  692,
      184, 933, 109, 427, 55,  352, 79,  646, 194, 901, 841, 208, 467, 801, 957,
      308, 426, 925, 861, 716, 691, 805, 436, 989, 90,  466, 721, 142, 801, 399,
      818, 880, 767, 788, 979, 908, 936, 648, 53,  645, 364, 782, 986, 332, 547,
      927, 244, 349, 161, 483, 955, 82,  532, 811, 688, 885, 290, 484, 351, 709,
      809, 118, 937, 958, 427, 489, 62,  666, 173, 520, 188, 947, 151, 528, 385,
      797, 725, 13,  448, 221, 638, 558, 446, 129, 458, 717, 514, 575, 886, 422,
      920, 276, 895, 170, 94,  656, 443, 838, 809, 194, 995, 337, 705, 902, 550,
      38,  666, 549, 65,  45,  194, 335, 941, 667, 719, 984, 493, 45,  502, 936,
      111, 338, 909, 61,  927, 378, 285, 537, 74,  355, 145, 290, 493, 975, 217,
      294, 132, 749, 903, 756, 306, 234, 634, 901, 781, 502, 40,  994, 691, 103,
      29,  265, 767, 407, 906, 198, 596, 375, 290, 760, 661, 635, 502, 561, 67,
      451, 492, 439, 450, 983, 589, 850, 350, 806, 201, 811, 741, 251, 466, 447,
      785, 955, 63,  936, 461, 124, 534, 968, 472, 410, 307, 58,  13,  15,  252,
      818, 782, 585, 206, 137, 510, 471, 50,  342, 910, 677, 910, 1,   115, 38,
      627, 484, 19,  531, 347, 653, 419, 201, 348, 541, 975, 58,  309, 855, 919,
      566, 898, 196, 260, 392, 38,  698, 995, 499, 654, 306, 640, 887, 516, 64,
      598, 198, 305, 511, 232, 961, 79,  623, 323, 870, 972, 28,  977, 392, 654,
      392, 366, 548, 486, 342, 673, 466, 590, 321, 765, 329, 356, 257, 761, 427,
      747, 178, 250, 824, 301, 97,  642, 688, 132, 992, 813, 699, 140, 542, 254,
      849, 527, 686, 332, 147, 922, 673, 480, 587, 915, 612, 945, 675, 100, 200,
      221, 146, 176, 116, 917, 717, 190, 584, 826, 645, 896, 643, 63,  292, 321,
      644, 70,  914, 643, 291, 869, 480, 716, 624, 707, 854, 81,  645, 333, 734,
      905, 284, 14,  506, 661, 658, 7,   864, 573, 530, 448, 732, 881, 41,  501,
      962, 925, 356, 691, 269, 505, 584, 540, 368, 655, 562, 211, 240, 759, 124,
      126, 233, 199, 176, 134, 574, 924, 280, 485, 652, 216, 674, 517, 467, 968,
      197, 653, 276, 557, 517, 627, 306, 85,  30,  789, 161, 299, 940, 477, 82,
      408, 219, 72,  825, 889, 660, 589, 430, 347, 469, 279, 386, 71,  320, 571,
      947, 639, 409, 518, 309, 700, 71,  405, 846, 522, 521, 873, 704, 805, 78,
      405, 34,  518, 370, 65,  117, 284, 663, 36,  409, 276, 461, 262, 77,  762,
      123, 757, 831, 118, 829, 742, 50,  474, 111, 740, 826, 373, 576, 960, 722,
      158, 500, 952, 956, 284, 772, 142, 769, 185, 722, 464, 668, 862, 316, 621,
      333, 290, 212, 351, 352, 2,   692, 942, 565, 996, 535, 957, 583, 970, 549,
      313, 922, 599, 839, 306, 101, 190, 177, 199, 307, 907, 877, 785, 786, 605,
      575, 576, 62,  53,  977, 689, 304, 818, 692, 691, 677, 677, 662, 68,  414,
      96,  9,   339, 981, 345, 522, 5,   822, 37,  865, 344, 249, 142, 944, 699,
      859, 276, 1,   629, 353, 17,  314, 962, 871, 476, 991, 577, 389, 750, 873,
      166, 977, 673, 525, 537, 449, 335, 373, 279, 787, 321, 841, 975, 344, 109,
      216, 344, 848, 889, 941, 451, 693, 520, 94,  969, 262, 783, 24,  311, 945,
      587, 376, 887, 215, 630, 107, 751, 940, 570, 748, 651, 605, 50,  55,  955,
      843, 173, 605, 814, 564, 671, 973, 679, 274, 815, 530, 162, 408, 37,  944,
      99,  401, 938, 726, 889, 676, 117, 685, 446, 741, 329, 922, 767, 906, 662,
      839, 195, 107, 925, 299, 536, 681, 412, 217, 499, 317, 679, 254, 622, 44,
      306, 523, 181, 558, 586, 772, 748, 834, 764, 547, 722, 392, 521, 235, 400,
      480, 702, 803, 67,  278, 804, 700, 473, 567, 71,  45,  629, 7,   65,  717,
      642, 458, 456, 214, 863, 524, 821, 79,  917, 132, 190, 364, 915, 975, 28,
      940, 669, 171, 600, 102, 969, 263, 81,  226, 861, 17,  850, 170, 513, 370,
      520, 3,   475, 988, 858, 114, 910, 19,  641, 823, 431, 905, 694, 14,  76,
      854, 469, 770, 82,  712, 702, 631, 802, 243, 358, 965, 314, 749, 847, 155,
      621, 928, 225, 532, 481, 632, 954, 511, 155, 690, 749, 142, 591, 489, 527,
      416, 154, 180, 277, 755, 773, 124, 199, 301, 600, 610, 554, 911, 916, 136,
      535, 417, 938, 210, 826, 151, 997, 462, 364, 38,  958, 24,  734, 349, 541,
      850, 374, 214, 894, 530, 294, 2,   424, 517, 569, 128, 274, 886, 412, 88,
      954, 408, 673, 296, 327, 365, 355, 269, 899, 377, 643, 730, 735, 51,  823,
      63,  42,  112, 425, 844, 746, 123, 662, 99,  606, 867, 516, 560, 461, 997,
      220, 876, 166, 326, 535, 776, 907, 485, 960, 277, 12,  236, 860, 860, 326,
      878, 251, 206, 735, 77,  720, 10,  551, 408, 672, 604, 483, 621, 519, 360,
      769, 68,  463, 392, 558, 194, 959, 949, 75,  374, 801, 631, 926, 39,  210,
      373, 827, 633, 531, 195, 419, 873, 567, 359, 961, 551, 660, 113, 180, 817,
      491, 643, 57,  478, 117, 831, 31,  278, 933, 675, 676, 424, 267, 755, 709,
      784, 62,  413, 260, 556, 472, 590, 448, 211, 978, 543, 640, 709, 864, 770,
      836, 974, 859, 406, 254, 859, 423, 672, 232, 584, 695, 379, 628, 255, 84,
      526, 16,  598, 763, 736, 17,  654, 414, 547, 378, 601, 887, 300, 659, 826,
      957, 284, 834, 297, 702, 682, 432, 428, 4,   15,  310, 773, 545, 137, 850,
      83,  425, 301, 505, 757, 582, 320, 742, 406, 703, 880, 283, 219, 122, 205,
      932, 899, 232, 189, 605, 359, 195, 745, 206, 823, 333, 570, 896, 673, 970,
      125, 849, 452, 112, 659, 570, 638, 147, 819, 402, 397, 733, 657, 171, 574,
      702, 994, 14,  592, 36,  536, 445, 174, 692, 247, 557, 741, 78,  962, 475,
      49,  724, 840, 786, 338, 539, 111, 101, 243, 815, 414, 287, 930, 842, 39,
      146, 120, 405, 829, 586, 183, 535, 871, 544, 28,  36,  969, 218, 209, 275,
      114, 269, 38,  941, 618, 899, 128, 570, 403, 52,  949, 954, 987, 754, 173,
      41,  747, 555, 93,  541, 136, 401, 935, 630, 477, 639, 416, 172, 830, 167,
      42,  897, 269, 725, 654, 456, 594, 754, 397, 61,  372, 303, 670, 814, 101,
      550, 656, 351, 156, 444, 689, 289, 418, 766, 303, 13,  912, 916, 896, 265,
      233, 878, 625, 852, 987, 567, 737, 529, 69,  913, 812, 193, 964, 439, 12,
      546, 685, 320, 733, 455, 508, 585, 734, 801, 71,  391, 183, 108, 810, 685,
      840, 10,  998, 861, 795, 40,  499, 234, 152, 797, 233, 480, 700, 242, 376,
      281, 861, 520, 421, 574, 163, 196, 422, 878, 872, 71,  62,  861, 868, 754,
      74,  760, 890, 113, 460, 512, 624, 548, 298, 664, 835, 102, 687, 784, 657,
      142, 638, 785, 444, 628, 389, 426, 810, 872, 192, 57,  576, 798, 107, 95,
      22,  847, 355, 611, 278, 553, 691, 289, 218, 139, 590, 789, 513, 308, 370,
      530, 625, 939, 328, 281, 789, 149, 800, 582, 89,  572, 986, 529, 325, 377,
      444, 654, 548, 240, 259, 362, 317, 698, 915, 718, 719, 134, 781, 352, 825,
      339, 69,  714, 967, 855, 68,  230, 285, 854, 260, 793, 855, 607, 595, 856,
      867, 49,  271, 428, 942, 988, 627, 667, 257, 858, 227, 91,  933, 111, 343,
      218, 805, 708, 505, 392, 646, 336, 20,  11,  670, 625, 72,  847, 697, 32,
      138, 574, 535, 797, 691, 455, 275, 96,  746, 200, 105, 244, 271, 702, 603,
      980, 447, 538, 2,   825, 663, 352, 949, 797, 457, 592, 10,  728, 397, 275,
      363, 998, 325, 506, 389, 176, 784, 348, 122, 599, 844, 281, 955, 733, 415,
      760, 393, 105, 659, 191, 700, 899, 90,  258, 180, 225, 93,  205, 463, 485,
      480, 99,  979, 296, 44,  76,  658, 535, 421, 755, 24,  268, 688, 266, 729,
      961, 453, 876, 448, 208, 591, 488, 947, 476, 571, 217, 837, 28,  257, 2,
      980, 339, 101, 95,  193, 735, 298, 66,  447, 758, 729, 827, 679, 51,  255,
      944, 917, 334, 470, 351, 58,  550, 59,  614, 431, 833, 587, 648, 709, 767,
      145, 69,  996, 367, 940, 662, 199, 693, 507, 42,  55,  755, 272, 139, 90,
      617, 941, 246, 392, 535, 181, 508, 153, 107, 811, 321, 806, 110, 403, 733,
      564, 696, 811, 889, 975, 931, 445, 509, 987, 32,  536, 905, 52,  468, 163,
      609, 723, 169, 344, 496, 780, 575, 154, 927, 352, 647, 693, 938, 690, 684,
      427, 961, 449, 848, 934, 936, 603, 909, 948, 792, 653, 191, 129, 896, 407,
      257, 306, 292, 168, 220, 982, 583, 918, 387, 3,   18,  452, 519, 733, 832,
      242, 917, 369, 568, 279, 355, 189, 131, 280, 631, 301, 32,  269, 331, 637,
      669, 604, 234, 824, 930, 530, 377, 201, 171, 760, 643, 585, 169, 697, 441,
      796, 729, 25,  37,  391, 857, 751, 907, 52,  539, 653, 476, 207, 390, 444,
      233, 828, 370, 975, 80,  91,  960, 168, 32,  976, 815, 674, 755, 254, 369,
      772, 919, 651, 280, 889, 46,  861, 79,  920, 256, 218, 142, 854, 826, 839,
      415, 435, 563, 493, 999, 18,  925, 968, 740, 672, 3,   302, 880, 512, 699,
      689, 354, 496, 217, 956, 839, 433, 540, 270, 301, 836, 176, 823, 436, 337,
      182, 283, 710, 408, 743, 240, 950, 740, 999, 109, 925, 358, 756, 72,  729,
      406, 448, 598, 797, 81,  9,   420, 965, 519, 350, 147, 93,  830, 99,  619,
      534, 764, 170, 27,  980, 339, 914, 857, 193, 391, 125, 691, 2,   522, 916,
      449, 232, 372, 611, 854, 102, 350, 600, 93,  563, 751, 331, 595, 880, 649,
      652, 590, 245, 58,  528, 49,  669, 557, 849, 96,  314, 291, 704, 280, 670,
      641, 675, 518, 854, 102, 268, 387, 532, 183, 731, 868, 231, 58,  148, 858,
      51,  921, 606, 296, 55,  344, 736, 914, 923, 124, 645, 294, 929, 988, 964,
      737, 557, 530, 815, 356, 587, 281, 573, 76,  29,  487, 278, 55,  629, 487,
      727, 399, 511, 727, 129, 760, 260, 102, 390, 482, 655, 124, 238, 696, 865,
      250, 478, 602, 918, 671, 274, 278, 495, 151, 988, 920, 852, 18,  247, 755,
      867, 895, 668, 350, 489, 536, 169, 855, 448, 180, 34,  691, 330, 274, 284,
      382, 206, 196, 154, 40,  218, 646, 477, 267, 352, 440, 28,  735, 983, 786,
      712, 335, 319, 793, 579, 590, 767, 466, 454, 243, 227, 535, 216, 945, 421,
      502, 349, 136, 4,   724, 190, 27,  925, 975, 996, 234, 922, 971, 92,  909,
      875, 862, 785, 192, 828, 163, 99,  396, 983, 598, 160, 139, 135, 884, 201,
      989, 315, 728, 990, 878, 981, 433, 21,  552, 676, 436, 722, 99,  993, 500,
      797, 230, 516, 938, 305, 249, 217, 668, 634, 162, 552, 767, 869, 240, 170,
      790, 217, 216, 879, 466, 361, 865, 920, 125, 978, 780, 158, 97,  489, 623,
      33,  50,  136, 313, 524, 796, 366, 278, 762, 581, 662, 27,  79,  908, 976,
      504, 64,  932, 14,  939, 608, 91,  186, 38,  574, 26,  629, 711, 9,   858,
      109, 594, 712, 744, 773, 984, 426, 383, 949, 333, 630, 426, 628, 64,  855,
      682, 725, 495, 157, 658, 971, 637, 328, 547, 694, 537, 656, 481, 993, 156,
      155, 973, 872, 346, 209, 414, 15,  361, 399, 183, 374, 276, 664, 460, 987,
      705, 186, 539, 270, 751, 379, 407, 52,  908, 703, 492, 28,  569, 464, 738,
      213, 382, 515, 638, 340, 708, 949, 659, 776, 526, 192, 773, 647, 629, 512,
      852, 932, 383, 512, 341, 10,  574, 641, 917, 439, 552, 849, 218, 682, 563,
      580, 493, 566, 303, 386, 457, 23,  464, 455, 145, 890, 544, 326, 882, 523,
      369, 658, 229, 819, 750, 60,  622, 626, 253, 47,  554, 514, 807, 931, 170,
      330, 416, 750, 750, 36,  297, 914, 80,  589, 910, 455, 938, 843, 910, 54,
      30,  86,  846, 714, 543, 503, 348, 903, 679, 4,   736, 565, 909, 642, 931,
      291, 236, 169, 696, 811, 824, 757, 571, 853, 480, 206, 594, 944, 12,  818,
      518, 370, 639, 658, 740, 760, 209, 733, 519, 807, 130, 388, 623, 122, 999,
      198, 486, 590, 816, 595, 297, 935, 963, 111, 875, 869, 49,  970, 917, 791,
      441, 860, 842, 149, 222, 320, 570, 522, 987, 707, 841, 863, 281, 634, 262,
      838, 67,  900, 459, 379, 768, 19,  854, 822, 78,  168, 201, 942, 413, 915,
      898, 913, 993, 264, 261, 492, 67,  128, 509, 154, 582, 800, 43,  641, 195,
      895, 303, 143, 312, 924, 629, 996, 663, 794, 234, 263, 114, 162, 565, 954,
      272, 786, 138, 232, 481, 294, 596, 94,  996, 71,  361, 724, 809, 211, 593,
      166, 325, 150, 964, 910, 371, 642, 761, 196, 306, 556, 211, 882, 672, 220,
      768, 32,  981, 757, 188, 894, 163, 449, 323, 362, 864, 376, 584, 297, 540,
      878, 465, 741, 86,  801, 767, 242, 347, 125, 989, 807, 577, 71,  480, 148,
      264, 2,   879, 900, 307, 830, 68,  727, 582, 436, 623, 751, 45,  317, 389,
      731, 550, 290, 834, 830, 483, 279, 875, 676, 441, 150, 574, 463, 171, 83,
      179, 242, 762, 677, 35,  291, 970, 667, 635, 37,  728, 742, 648, 229, 809,
      352, 136, 673, 671, 975, 240, 60,  540, 248, 796, 8,   927, 718, 504, 685,
      907, 30,  55,  555, 725, 529, 681, 848, 869, 66,  762, 829, 200, 662, 380,
      969, 861, 938, 271, 665, 640, 499, 196, 998, 509, 118, 105, 475, 397, 355,
      784, 84,  687, 99,  180, 458, 55,  957, 81,  195, 107, 176, 792, 92,  139,
      232, 91,  464, 816, 767, 238, 173, 734, 865, 236, 115, 122, 278, 853, 945,
      597, 338, 397, 365, 419, 431, 199, 433, 88,  381, 163, 730, 709, 736, 550,
      418, 929, 490, 38,  717, 767, 264, 37,  775, 260, 412, 105, 332, 921, 385,
      745, 999, 798, 93,  45,  353, 303, 697, 506, 643, 910, 138, 634, 898, 660,
      577, 325, 819, 247, 658, 840, 301, 906, 733, 111, 634, 636, 826, 102, 857,
      186, 450, 486, 920, 647, 348, 911, 471, 785, 945, 779, 706, 512, 376, 58,
      68,  266, 227, 203, 893, 574, 94,  836, 573, 872, 470, 712, 760, 471, 156,
      753, 734, 130, 293, 319, 614, 734, 691, 664, 968, 51,  352, 160, 460, 612,
      748, 676, 525, 967, 590, 671, 260, 751, 275, 78,  596, 571, 304, 600, 351,
      242, 485, 705, 989, 167, 158, 412, 314, 647, 539, 338, 937, 269, 778, 751,
      903, 1,   480, 118, 855, 863, 300, 683, 673, 457, 191, 515, 96,  853, 512,
      649, 317, 932, 413, 105, 937, 323, 907, 837, 46,  670, 574, 613, 760, 316,
      735, 76,  177, 435, 362, 68,  104, 885, 342, 274, 783, 952, 725, 20,  97,
      537, 99,  517, 135, 694, 735, 337, 208, 99,  730, 101, 572, 836, 822, 715,
      326, 873, 604, 288, 671, 361, 605, 845, 459, 424, 135, 910, 36,  134, 451,
      698, 784, 213, 377, 219, 475, 42,  738, 538, 762, 741, 669, 897, 744, 915,
      592, 540, 429, 239, 19,  294, 852, 539, 370, 145, 794, 237, 669, 95,  338,
      245, 946, 760, 358, 498, 844, 38,  245, 400, 294, 55,  996, 212, 167, 275,
      119, 636, 871, 222, 539, 14,  512, 59,  339, 273, 301, 627, 394, 828, 798,
      619, 556, 539, 718, 278, 580, 870, 740, 160, 721, 58,  428, 847, 710, 528,
      30,  757, 692, 812, 819, 960, 224, 70,  32,  714, 153, 946, 143, 705, 70,
      174, 567, 185, 86,  378, 832, 988, 451, 382, 990, 771, 598, 191, 404, 359,
      137, 814, 729, 943, 466, 412, 843, 157, 735, 732, 774, 496, 852, 733, 402,
      345, 116, 771, 62,  829, 364, 606, 355, 974, 818, 446, 211, 318, 296, 240,
      18,  319, 437, 84,  612, 845, 695, 949, 670, 938, 750, 61,  462, 83,  9,
      478, 616, 784, 263, 994, 314, 153, 403, 689, 926, 84,  339, 419, 160, 855,
      294, 206, 93,  206, 999, 362, 346, 447, 618, 150, 784, 238, 33,  173, 883,
      118, 36,  843, 52,  795, 534, 134, 685, 49,  629, 556, 776, 909, 99,  66,
      648, 545, 490, 100, 995, 743, 115, 893, 956, 826, 577, 410, 865, 701, 707,
      457, 379, 547, 130, 702, 52,  528, 208, 486, 788, 356, 136, 814, 548, 331,
      285, 578, 627, 307, 520, 532, 757, 62,  294, 255, 941, 681, 235, 823, 520,
      998, 463, 577, 422, 313, 402, 321, 920, 499, 251, 391, 352, 776, 604, 70,
      346, 824, 418, 47,  569, 734, 566, 380, 984, 749, 611, 187, 783, 595, 128,
      273, 970, 756, 379, 777, 135, 82,  22,  204, 666, 375, 76,  181, 889, 864,
      374, 838, 718, 141, 139, 361, 327, 56,  880, 683, 510, 458, 17,  144, 116,
      636, 99,  561, 329, 967, 420, 14,  942, 289, 777, 40,  81,  207, 29,  835,
      157, 187, 354, 792, 755, 184, 725, 927, 805, 19,  693, 800, 121, 810, 867,
      814, 887, 905, 391, 803, 263, 597, 968, 475, 453, 5,   898, 817, 464, 587,
      442, 961, 734, 570, 773, 680, 107, 187, 29,  916, 851, 935, 867, 603, 128,
      430, 361, 204, 873, 315, 218, 612, 746, 919, 735, 461, 730, 236, 485, 61,
      349, 975, 820, 289, 837, 198, 698, 184, 704, 40,  620, 10,  159, 281, 224,
      368, 418, 676, 539, 394, 156, 692, 127, 412, 200, 918, 608, 962, 448, 962,
      580, 292, 941, 590, 665, 939, 178, 883, 236, 643, 717, 7,   671, 392, 352,
      855, 376, 124, 216, 493, 126, 752, 758, 845, 719, 188, 239, 35,  177, 641,
      894, 148, 459, 474, 546, 981, 952, 869, 554, 596, 470, 652, 812, 414, 22,
      82,  4,   536, 156, 859, 823, 464, 302, 998, 261, 187, 891, 201, 289, 655,
      126, 217, 213, 591, 343, 941, 238, 5,   572, 348, 21,  275, 321, 518, 19,
      477, 867, 332, 251, 83,  7,   184, 952, 169, 268, 456, 317, 811, 607, 241,
      913, 190, 443, 310, 295, 524, 678, 429, 659, 66,  929, 802, 473, 879, 494,
      15,  665, 695, 875, 36,  551, 162, 504, 769, 259, 857, 771, 813, 104, 338,
      219, 646, 60,  16,  793, 395, 807, 320, 905, 498, 253, 561, 852, 927, 501,
      239, 223, 847, 691, 439, 709, 763, 11,  754, 718, 643, 146, 647, 123, 279,
      828, 133, 491, 218, 336, 33,  356, 706, 919, 326, 699, 968, 97,  317, 375,
      885, 420, 495, 996, 540, 92,  26,  76,  511, 240, 505, 818, 56,  412, 814,
      791, 855, 292, 654, 86,  817, 853, 665, 858, 323, 986, 468, 804, 913, 927,
      11,  62,  382, 765, 987, 932, 316, 944, 622, 319, 550, 790, 60,  524, 743,
      662, 65,  26,  654, 863, 140, 586, 128, 131, 786, 619, 937, 737, 474, 737,
      570, 710, 773, 591, 700, 224, 581, 697, 149, 451, 382, 667, 597, 681, 631,
      534, 629, 142, 412, 813, 661, 47,  972, 224, 56,  235, 66,  348, 854, 800,
      978, 421, 507, 799, 122, 551, 194, 964, 663, 561, 627, 877, 291, 573, 777,
      947, 52,  832, 71,  139, 107, 485, 467, 214, 930, 725, 231, 607, 928, 331,
      824, 244, 733, 134, 86,  42,  454, 855, 278, 248, 69,  476, 328, 512, 395,
      246, 806, 968, 651, 667, 141, 938, 926, 978, 605, 470, 171, 124, 420, 726,
      99,  223, 603, 885, 315, 374, 285, 347, 956, 674, 146, 993, 166, 189, 34,
      838, 714, 42,  741, 397, 35,  227, 576, 485, 772, 561, 564, 328, 40,  277,
      229, 113, 529, 584, 375, 731, 945, 301, 156, 610, 999, 291, 859, 733, 423,
      980, 535, 891, 718, 709, 780, 499, 426, 565, 171, 52,  891, 136, 122, 2,
      764, 395, 595, 352, 736, 631, 818, 130, 745, 200, 732, 560, 553, 874, 384,
      687, 307, 931, 604, 370, 609, 657, 624, 117, 690, 601, 258, 915, 795, 224,
      755, 987, 156, 383, 462, 217, 885, 329, 47,  188, 180, 568, 324, 864, 92,
      202, 752, 192, 823, 673, 246, 622, 77,  822, 213, 368, 907, 210, 965, 521,
      326, 172, 374, 812, 555, 147, 74,  347, 822, 625, 745, 630, 95,  82,  954,
      967, 197, 738, 256, 891, 871, 580, 128, 466, 402, 144, 535, 588, 556, 901,
      533, 478, 722, 638, 860, 157, 129, 357, 338, 994, 554, 407, 113, 8,   594,
      55,  827, 747, 73,  102, 867, 422, 873, 790, 5,   639, 896, 645, 587, 160,
      440, 754, 161, 533, 477, 884, 873, 60,  193, 874, 726, 81,  554, 740, 966,
      620, 460, 754, 3,   572, 852, 168, 165, 324, 428, 600, 139, 677, 342, 180,
      766, 567, 1,   32,  125, 640, 553, 732, 274, 165, 257, 33,  461, 201, 953,
      183, 801, 754, 617, 279, 812, 100, 486, 976, 101, 514, 765, 455, 592, 629,
      71,  62,  717, 487, 140, 126, 948, 892, 518, 614, 812, 458, 455, 653, 282,
      61,  999, 921, 55,  399, 960, 427, 907, 496, 220, 835, 406, 385, 429, 995,
      141, 192, 327, 569, 366, 41,  923, 818, 929, 382, 659, 205, 108, 971, 607,
      364, 987, 656, 147, 551, 178, 905, 609, 347, 349, 484, 48,  547, 525, 266,
      172, 648, 330, 812, 840, 382, 888, 47,  782, 146, 959, 929, 882, 681, 501,
      874, 490, 8,   280, 446, 888, 970, 246, 472, 600, 428, 367, 236, 608, 958,
      41,  58,  525, 860, 793, 878, 97,  340, 881, 464};

  int64_t keys2[] = {
      544, 965, 330, 59,  367, 744, 929, 461, 148, 350, 154, 688, 331, 188,
      927, 466, 937, 780, 977, 652, 878, 709, 13,  833, 373, 796, 81,  115,
      878, 601, 714, 891, 232, 438, 553, 105, 732, 336, 139, 215, 951, 825,
      807, 639, 363, 968, 997, 235, 166, 762, 831, 299, 832, 577, 15,  545,
      398, 913, 614, 346, 564, 431, 905, 39,  21,  625, 658, 139, 980, 680,
      714, 132, 314, 532, 518, 643, 493, 321, 912, 577, 940, 347, 279, 253,
      901, 495, 868, 581, 234, 370, 553, 208, 132, 722, 116, 587, 543, 473,
      327, 884, 61,  634, 654, 41,  755, 554, 818, 255, 891, 171, 936, 127,
      516, 426, 727, 623, 530, 748, 444, 427, 756, 932, 640, 749, 995, 159,
      56,  179, 906, 117, 943, 947, 613, 753, 620, 556, 122, 926, 397, 579,
      551, 799, 308, 41,  307, 431, 427, 160, 611, 970, 189, 171, 277, 66,
      353, 439, 304, 348, 587, 244, 305, 491, 561, 521, 845, 912, 488, 456,
      23,  785, 62,  725, 481, 461, 277, 959, 813, 446, 917, 408, 318, 143,
      714, 869, 867, 280, 627, 141, 427, 307, 503, 614, 797, 271, 966, 759,
      978, 288, 541, 178, 870, 601, 505, 156, 960, 296, 848, 364, 978, 381,
      748, 253, 50,  499, 731, 525, 813, 447, 361, 266, 412, 487, 192, 212,
      38,  120, 598, 749, 854, 48,  941, 18,  478, 700, 127, 374, 366, 310,
      417, 100, 413, 670, 466, 12,  652, 832, 252, 814, 56,  845, 34,  212,
      655, 722, 956, 771, 721, 921, 432, 909, 385, 61,  46,  646, 176, 762,
      999, 102, 764, 766, 302, 778, 440, 737, 5,   392, 243, 122, 667, 961,
      637, 132, 943, 565, 203, 562, 746, 505, 160, 812, 437, 444, 708, 774,
      192, 877, 9,   754, 692, 556, 87,  461, 703, 35,  204, 565, 413, 13,
      825, 690, 895, 71,  590, 352, 921, 339, 162, 275, 556, 105, 858, 642,
      661, 77,  181, 639, 369, 59,  624, 300, 852, 508, 982, 645, 371, 851,
      600, 431, 957, 732, 35,  385, 101, 338, 696, 814, 188, 801, 126, 278,
      51,  120, 165, 941, 157, 53,  257, 265, 872, 462, 91,  236, 854, 587,
      587, 556, 991, 904, 991, 796, 565, 599, 447, 87,  783, 124, 596, 707,
      691, 98,  67,  94,  159, 823, 68,  133, 150, 955, 426, 95,  48,  730,
      364, 992, 446, 206, 556, 268, 34,  550, 206, 292, 442, 985, 793, 48,
      980, 227, 578, 958, 175, 931, 592, 577, 66,  801, 698, 819, 476, 2,
      607, 742, 574, 93,  639, 736, 613, 669, 899, 601, 497, 57,  953, 322,
      879, 294, 237, 216, 924, 497, 339, 898, 167, 413, 833, 133, 481, 779,
      379, 76,  597, 306, 385, 73,  107, 928, 446, 913, 531, 152, 123, 267,
      73,  294, 858, 257, 668, 139, 349, 591, 474, 769, 552, 68,  637, 13,
      381, 874, 45,  195, 409, 53,  414, 422, 313, 305, 475, 343, 944, 911,
      765, 192, 32,  50,  704, 926, 77,  245, 642, 37,  666, 309, 816, 56,
      524, 876, 530, 683, 956, 167, 962, 367, 693, 306, 701, 641, 297, 955,
      374, 883, 668, 67,  290, 284, 800, 590, 998, 612, 768, 890, 28,  725,
      719, 497, 767, 282, 756, 974, 867, 736, 635, 833, 841, 402, 65,  57,
      32,  297, 505, 943, 283, 662, 780, 427, 191, 319, 945, 773, 512, 7,
      90,  170, 153, 358, 966, 584, 249, 925, 528, 836, 809, 300, 385, 359,
      376, 189, 65,  312, 841, 515, 976, 632, 24,  625, 405, 788, 359, 109,
      879, 311, 876, 264, 54,  255, 664, 635, 505, 948, 80,  806, 253, 151,
      993, 677, 5,   979, 286, 689, 234, 289, 297, 277, 823, 472, 321, 730,
      74,  389, 461, 947, 430, 914, 98,  587, 766, 509, 15,  3,   964, 998,
      951, 672, 39,  973, 474, 604, 875, 73,  870, 678, 921, 399, 631, 842,
      403, 656, 853, 44,  206, 243, 290, 175, 590, 498, 173, 608, 392, 152,
      125, 308, 125, 463, 416, 609, 604, 590, 113, 389, 884, 707, 142, 315,
      296, 171, 515, 360, 451, 824, 556, 804, 479, 494, 662, 614, 297, 481,
      574, 896, 108, 14,  549, 440, 983, 460, 87,  511, 370, 250, 5,   31,
      984, 382, 712, 727, 395, 8,   612, 778, 700, 201, 569, 298, 464, 126,
      889, 64,  122, 500, 113, 154, 361, 106, 448, 613, 331, 768, 849, 601,
      98,  557, 812, 499, 962, 36,  772, 492, 574, 578, 716, 511, 222, 622,
      547, 966, 720, 78,  948, 249, 185, 246, 12,  989, 620, 757, 125, 298,
      918, 539, 566, 47,  77,  165, 101, 877, 687, 758, 679, 683, 491, 636,
      382, 127, 157, 755, 158, 744, 602, 607, 397, 184, 722, 860, 492, 404,
      964, 952, 689, 614, 389, 713, 322, 835, 215, 264, 851, 62,  414, 453,
      924, 966, 312, 187, 608, 656, 243, 454, 509, 525, 77,  101, 449, 118,
      671, 117, 903, 116, 846, 521, 162, 744, 203, 605, 998, 667, 742, 930,
      226, 649, 552, 461, 795, 380, 162, 330, 312, 961, 950, 427, 381, 100,
      898, 158, 582, 869, 501, 288, 250, 922, 664, 168, 101, 988, 969, 665,
      970, 298, 849, 604, 616, 311, 136, 469, 780, 750, 59,  181, 448, 495,
      575, 887, 202, 429, 531, 907, 220, 26,  343, 30,  573, 781, 852, 347,
      921, 932, 609, 840, 846, 272, 631, 416, 34,  608, 111, 7,   151, 934,
      660, 196, 509, 682, 702, 367, 446, 197, 318, 38,  418, 884, 218, 464,
      1,   923, 595, 199, 327, 666, 719, 920, 541, 555, 50,  797, 714, 526,
      637, 497, 345, 694, 45,  722, 126, 663, 961, 43,  487, 160, 183, 67,
      282, 637, 673, 317, 344, 16,  506, 953, 307, 817, 133, 532, 862, 145,
      981, 178, 74,  415, 687, 909, 458, 568, 765, 488, 174, 313, 244, 180,
      108, 357, 906, 804, 919, 733, 594, 789, 618, 818, 468, 284, 40,  105,
      876, 356, 954, 186, 272, 376, 361, 193, 142, 981, 774, 149, 296, 2,
      127, 565, 246, 47,  314, 162, 228, 966, 393, 834, 23,  886, 449, 85,
      133, 440, 259, 710, 70,  850, 295, 117, 871, 875, 486, 370, 435, 407,
      77,  534, 597, 534, 781, 849, 46,  631, 388, 913, 559, 469, 273, 65,
      929, 413, 188, 127, 738, 633, 825, 602, 392, 503, 973, 863, 319, 650,
      129, 919, 322, 299, 733, 464, 499, 696, 839, 251, 997, 242, 428, 799,
      36,  996, 382, 531, 886, 979, 541, 361, 858, 269, 881, 231, 77,  218,
      706, 968, 620, 661, 200, 879, 629, 103, 310, 197, 551, 796, 989, 809,
      748, 809, 339, 563, 84,  31,  115, 690, 915, 15,  396, 614, 918, 411,
      279, 516, 66,  691, 779, 980, 793, 913, 432, 510, 600, 659, 587, 770,
      505, 456, 354, 726, 506, 390, 915, 411, 958, 824, 58,  373, 302, 409,
      469, 186, 688, 909, 415, 83,  3,   641, 633, 100, 956, 394, 899, 321,
      623, 788, 606, 402, 24,  538, 45,  134, 292, 657, 693, 116, 13,  205,
      593, 703, 163, 611, 104, 171, 184, 613, 471, 377, 945, 929, 997, 810,
      596, 8,   954, 283, 381, 131, 853, 847, 665, 650, 680, 308, 129, 698,
      13,  907, 643, 142, 682, 366, 355, 80,  197, 661, 809, 511, 289, 913,
      825, 438, 427, 269, 844, 312, 515, 540, 361, 575, 946, 874, 79,  442,
      566, 382, 893, 776, 620, 427, 721, 402, 348, 822, 645, 685, 478, 432,
      428, 82,  104, 554, 446, 274, 449, 900, 643, 454, 654, 550, 864, 656,
      545, 77,  787, 515, 148, 348, 753, 742, 715, 18,  206, 502, 755, 987,
      826, 511, 991, 543, 375, 200, 913, 661, 538, 977, 673, 520, 710, 557,
      474, 239, 361, 63,  687, 391, 358, 398, 80,  848, 570, 316, 368, 939,
      503, 745, 751, 169, 356, 439, 721, 801, 137, 446, 864, 361, 494, 554,
      967, 42,  252, 411, 160, 41,  100, 168, 687, 770, 420, 399, 357, 426,
      612, 211, 146, 929, 138, 888, 803, 618, 574, 722, 273, 811, 746, 630,
      835, 942, 171, 181, 296, 438, 326, 725, 165, 274, 645, 591, 571, 552,
      90,  178, 505, 307, 908, 696, 990, 437, 580, 685, 167, 484, 433, 235,
      126, 880, 120, 705, 252, 448, 870, 417, 64,  942, 55,  175, 238, 172,
      428, 171, 285, 180, 613, 38,  618, 3,   281, 207, 727, 808, 422, 390,
      642, 994, 517, 150, 127, 339, 911, 570, 931, 419, 468, 536, 308, 857,
      882, 656, 249, 24,  633, 191, 898, 934, 942, 52,  97,  318, 257, 384,
      848, 794, 229, 785, 870, 978, 392, 933, 644, 220, 326, 361, 150, 873,
      863, 133, 608, 153, 930, 251, 333, 623, 860, 12,  904, 855, 546, 46,
      440, 109, 537, 414, 164, 606, 672, 664, 971, 383, 525, 895, 364, 607,
      107, 948, 761, 548, 290, 773, 259, 689, 746, 682, 15,  856, 245, 67,
      630, 18,  683, 614, 43,  465, 764, 263, 276, 297, 813, 623, 651, 199,
      732, 892, 385, 503, 126, 283, 88,  308, 920, 586, 475, 512, 973, 417,
      940, 267, 359, 667, 405, 554, 171, 591, 125, 470, 437, 810, 428, 19,
      915, 57,  802, 850, 560, 94,  620, 467, 775, 343, 932, 849, 395, 584,
      741, 440, 776, 655, 192, 636, 133, 443, 234, 636, 108, 105, 50,  335,
      105, 765, 843, 11,  53,  156, 532, 414, 345, 516, 969, 826, 536, 990,
      690, 878, 214, 8,   92,  174, 901, 733, 201, 22,  799, 696, 126, 755,
      584, 174, 685, 54,  298, 717, 379, 251, 229, 461, 674, 893, 424, 348,
      306, 940, 645, 518, 967, 656, 530, 67,  912, 293, 291, 909, 645, 380,
      291, 178, 896, 2,   796, 93,  84,  334, 515, 738, 871, 406, 747, 21,
      278, 678, 217, 832, 816, 848, 727, 871, 464, 855, 279, 17,  318, 434,
      509, 429, 188, 304, 781, 112, 725, 714, 631, 48,  644, 545, 834, 144,
      77,  44,  465, 834, 796, 626, 671, 225, 553, 721, 582, 104, 872, 576,
      394, 174, 187, 94,  734, 381, 142, 198, 708, 871, 290, 964, 587, 68,
      315, 312, 95,  94,  372, 526, 308, 968, 372, 627, 591, 108, 390, 479,
      918, 551, 171, 726, 763, 245, 352, 8,   68,  599, 226, 570, 727, 22,
      977, 88,  842, 645, 538, 669, 419, 883, 487, 963, 770, 663, 642, 551,
      696, 804, 303, 190, 384, 359, 155, 497, 397, 971, 284, 914, 70,  124,
      899, 481, 868, 396, 958, 458, 818, 614, 629, 978, 124, 591, 170, 85,
      346, 843, 340, 850, 988, 781, 904, 234, 606, 368, 404, 974, 411, 578,
      739, 512, 629, 84,  177, 394, 463, 409, 818, 312, 456, 259, 637, 442,
      823, 222, 52,  629, 683, 215, 916, 266, 954, 475, 311, 997, 298, 39,
      691, 508, 607, 360, 754, 453, 510, 22,  171, 480, 228, 592, 503, 251,
      67,  800, 47,  164, 342, 40,  719, 892, 13,  285, 91,  892, 378, 235,
      646, 898, 465, 179, 166, 612, 477, 751, 57,  519, 907, 379, 510, 893,
      292, 581, 990, 724, 295, 447, 868, 651, 118, 583, 978, 884, 976, 155,
      241, 900, 458, 602, 108, 68,  261, 455, 19,  663, 256, 971, 607, 766,
      326, 545, 353, 624, 784, 786, 902, 90,  431, 287, 643, 687, 952, 82,
      441, 132, 764, 310, 237, 355, 134, 405, 361, 683, 884, 708, 712, 285,
      814, 647, 879, 767, 384, 196, 687, 158, 320, 476, 681, 842, 77,  8,
      214, 754, 917, 476, 994, 171, 536, 513, 15,  293, 641, 468, 455, 108,
      201, 890, 428, 175, 471, 885, 231, 981, 983, 321, 689, 77,  113, 650,
      389, 236, 480, 391, 260, 131, 140, 649, 657, 904, 595, 679, 77,  425,
      684, 605, 282, 365, 796, 366, 143, 625, 160, 221, 403, 184, 778, 377,
      806, 511, 855, 95,  695, 609, 56,  902, 295, 273, 267, 577, 308, 484,
      125, 948, 504, 363, 459, 974, 295, 814, 927, 908, 908, 299, 266, 547,
      339, 107, 42,  375, 2,   251, 573, 573, 786, 118, 534, 666, 687, 450,
      68,  400, 621, 733, 426, 650, 325, 391, 826, 963, 559, 771, 48,  204,
      5,   74,  451, 923, 133, 986, 541, 86,  822, 867, 541, 510, 561, 334,
      286, 93,  522, 231, 331, 379, 222, 630, 308, 291, 357, 47,  536, 985,
      562, 634, 26,  99,  675, 55,  121, 177, 418, 17,  667, 532, 457, 883,
      310, 143, 149, 30,  361, 146, 414, 99,  804, 64,  375, 650, 173, 95,
      672, 97,  571, 494, 807, 317, 344, 6,   374, 654, 429, 717, 935, 275,
      785, 461, 521, 851, 314, 441, 462, 932, 867, 626, 137, 23,  996, 676,
      95,  503, 717, 728, 155, 209, 543, 581, 82,  170, 337, 146, 920, 640,
      840, 942, 984, 11,  683, 570, 636, 141, 163, 99,  182, 616, 990, 798,
      69,  685, 243, 252, 338, 936, 677, 179, 106, 120, 303, 675, 702, 660,
      126, 841, 842, 663, 621, 56,  284, 577, 401, 64,  11,  645, 114, 753,
      738, 797, 790, 812, 96,  120, 762, 613, 794, 188, 929, 466, 860, 195,
      47,  353, 830, 98,  226, 32,  128, 504, 985, 994, 401, 711, 227, 866,
      863, 372, 503, 440, 92,  116, 730, 808, 30,  23,  75,  427, 782, 917,
      902, 227, 398, 928, 786, 997, 31,  191, 697, 99,  676, 227, 469, 573,
      878, 392, 819, 859, 519, 830, 865, 462, 900, 265, 523, 20,  550, 569,
      658, 969, 737, 897, 272, 169, 149, 244, 937, 340, 689, 665, 775, 55,
      723, 195, 20,  296, 914, 638, 340, 506, 918, 382, 969, 779, 821, 456,
      65,  925, 316, 773, 96,  831, 773, 430, 446, 222, 410, 113, 814, 693,
      419, 722, 550, 118, 212, 430, 445, 270, 701, 918, 800, 333, 377, 318,
      534, 642, 643, 326, 995, 975, 860, 275, 907, 957, 452, 623, 785, 587,
      265, 141, 639, 506, 391, 199, 939, 655, 891, 325, 37,  582, 55,  381,
      976, 284, 222, 570, 903, 112, 34,  682, 595, 9,   946, 820, 599, 836,
      10,  433, 799, 648, 423, 43,  566, 857, 283, 760, 610, 365, 115, 597,
      907, 325, 405, 676, 317, 798, 360, 506, 954, 901, 599, 938, 294, 59,
      109, 378, 979, 75,  444, 18,  130, 972, 779, 38,  673, 475, 879, 972,
      645, 26,  249, 893, 422, 442, 203, 42,  802, 464, 390, 973, 704, 346,
      522, 938, 420, 915, 483, 378, 571, 981, 132, 780, 583, 739, 634, 199,
      561, 792, 934, 453, 235, 469, 642, 307, 41,  489, 90,  74,  188, 291,
      980, 742, 16,  937, 438, 758, 676, 865, 627, 317, 323, 100, 494, 293,
      104, 403, 380, 161, 840, 453, 563, 622, 888, 707, 783, 922, 641, 596,
      278, 676, 658, 911, 908, 927, 662, 393, 735, 134, 472, 262, 289, 486,
      725, 351, 830, 105, 301, 97,  497, 722, 221, 252, 241, 407, 643, 614,
      393, 941, 209, 251, 942, 773, 156, 435, 272, 725, 29,  856, 213, 375,
      990, 41,  631, 637, 92,  126, 901, 293, 648, 104, 438, 846, 33,  858,
      812, 257, 872, 107, 655, 745, 629, 353, 170, 161, 691, 997, 362, 68,
      533, 820, 742, 912, 66,  58,  946, 823, 743, 230, 835, 182, 83,  423,
      703, 389, 166, 720, 249, 136, 958, 142, 440, 958, 976, 486, 40,  144,
      10,  666, 893, 427, 434, 868, 559, 691, 305, 666, 561, 763, 500, 284,
      644, 362, 58,  423, 32,  961, 323, 88,  477, 8,   411, 637, 60,  109,
      890, 314, 790, 820, 336, 324, 468, 183, 881, 471, 882, 593, 417, 423,
      575, 385, 390, 938, 852, 334, 922, 581, 804, 698, 235, 758, 533, 5,
      149, 926, 660, 345, 495, 499, 886, 894, 658, 278, 427, 673, 634, 736,
      951, 749, 38,  970, 983, 787, 321, 273, 388, 692, 601, 233, 686, 301,
      278, 709, 178, 455, 455, 334, 616, 358, 321, 679, 525, 363, 774, 942,
      89,  174, 518, 607, 910, 5,   299, 808, 775, 716, 425, 178, 441, 857,
      507, 159, 49,  970, 303, 718, 212, 784, 37,  321, 611, 509, 451, 940,
      15,  666, 446, 943, 145, 519, 472, 197, 787, 170, 496, 42,  715, 343,
      686, 278, 997, 317, 574, 820, 248, 547, 53,  238, 291, 773, 541, 348,
      726, 502, 540, 720, 143, 855, 987, 492, 797, 493, 374, 934, 217, 573,
      163, 991, 94,  53,  40,  896, 908, 503, 615, 414, 134, 815, 805, 425,
      343, 91,  962, 210, 203, 561, 277, 997, 915, 454, 576, 140, 250, 567,
      334, 105, 594, 5,   906, 681, 398, 785, 680, 479, 316, 346, 110, 88,
      464, 821, 644, 243, 298, 478, 570, 193, 825, 614, 814, 546, 891, 484,
      902, 27,  113, 720, 171, 116, 976, 700, 504, 570, 421, 350, 278, 182,
      535, 499, 103, 26,  3,   150, 197, 770, 605, 87,  348, 545, 100, 615,
      390, 959, 12,  785, 695, 454, 832, 580, 119, 456, 178, 791, 380, 708,
      917, 991, 995, 710, 312, 397, 98,  2,   379, 994, 597, 872, 765, 824,
      605, 417, 380, 897, 654, 893, 17,  183, 849, 156, 482, 507, 192, 943,
      684, 782, 875, 197, 314, 911, 501, 330, 764, 46,  790, 574, 979, 432,
      705, 336, 238, 760, 599, 161, 145, 429, 540, 645, 15,  247, 595, 262,
      957, 956, 15,  316, 308, 880, 662, 407, 33,  723, 155, 644, 554, 505,
      159, 110, 780, 780, 745, 593, 681, 203, 728, 312, 479, 183, 272, 973,
      846, 446, 965, 357, 592, 961, 56,  924, 972, 296, 584, 812, 25,  702,
      95,  600, 858, 390, 85,  980, 450, 175, 635, 348, 159, 114, 923, 722,
      698, 151, 648, 857, 455, 70,  63,  341, 401, 579, 181, 862, 29,  862,
      511, 397, 703, 964, 833, 839, 981, 625, 33,  695, 25,  512, 48,  516,
      955, 364, 912, 394, 629, 234, 458, 131, 369, 526, 942, 636, 973, 218,
      872, 297, 410, 321, 935, 897, 677, 39,  833, 328, 124, 114, 465, 338,
      366, 238, 126, 856, 533, 258, 998, 522, 592, 398, 450, 473, 293, 828,
      944, 25,  241, 551, 501, 699, 317, 539, 381, 856, 197, 672, 181, 427,
      167, 521, 299, 286, 305, 550, 74,  44,  503, 711, 474, 73,  642, 935,
      696, 251, 36,  830, 683, 234, 936, 912, 265, 336, 529, 536, 828, 325,
      169, 361, 959, 28,  431, 537, 746, 461, 748, 984, 450, 701, 663, 429,
      146, 711, 351, 312, 847, 796, 28,  522, 228, 53,  698, 97,  904, 483,
      908, 139, 198, 363, 118, 263, 790, 304, 914, 994, 533, 773, 24,  615,
      354, 348, 743, 312, 286, 671, 677, 734, 357, 239, 236, 347, 998, 781,
      528, 457, 152, 892, 312, 911, 617, 894, 265, 336, 157, 645, 54,  829,
      219, 412, 92,  236, 317, 6,   478, 207, 215, 434, 961, 947, 580, 890,
      847, 613, 877, 739, 964, 467, 201, 32,  955, 27,  439, 271, 785, 556,
      9,   968, 514, 368, 264, 193, 215, 501, 920, 522, 413, 600, 342, 419,
      125, 555, 606, 55,  825, 346, 674, 322, 349, 743, 919, 920, 624, 595,
      83,  198, 232, 541, 278, 473, 908, 256, 606, 273, 34,  572, 681, 283,
      187, 606, 341, 64,  719, 978, 667, 868, 382, 570, 288, 410, 292, 886,
      586, 326, 375, 740, 608, 116, 981, 850, 334, 35,  874, 929, 22,  125,
      626, 424, 923, 897, 190, 347, 675, 781, 832, 732, 517, 220, 178, 190,
      821, 17,  160, 275, 856, 246, 910, 18,  188, 353, 622, 731, 313, 651,
      620, 387, 31,  252, 404, 490, 113, 912, 667, 591, 740, 163, 163, 566,
      573, 742, 926, 476, 49,  191, 740, 591, 259, 512, 913, 145, 838, 46,
      49,  227, 487, 632, 384, 609, 665, 797, 727, 16,  140, 775, 608, 605,
      226, 751, 368, 799, 135, 46,  59,  771, 639, 149, 512, 598, 391, 277,
      778, 409, 350, 10,  782, 330, 971, 424, 98,  318, 120, 588, 204, 947,
      102, 720, 544, 462, 526, 959, 952, 858, 404, 723, 742, 475, 247, 563,
      618, 818, 89,  105, 916, 911, 76,  457, 777, 800, 660, 807, 689, 14,
      698, 656, 953, 221, 28,  986, 997, 46,  429, 839, 344, 936, 131, 887,
      530, 67,  76,  558, 397, 149, 291, 314, 233, 860, 689, 439, 782, 244,
      702, 429, 925, 431, 25,  528, 741, 236, 38,  758, 505, 938, 629, 191,
      967, 880, 870, 606, 2,   480, 216, 344, 637, 720, 382, 885, 298, 914,
      785, 980, 865, 361, 491, 241, 56,  454, 260, 42,  80,  145, 892, 402,
      832, 290, 818, 684, 332, 974, 677, 750, 281, 830, 833, 235, 903, 240,
      740, 778, 496, 998, 901, 691, 319, 948, 185, 896, 480, 63,  269, 300,
      824, 193, 29,  752, 293, 68,  948, 138, 385, 562, 892, 846, 614, 869,
      473, 802, 314, 912, 702, 440, 358, 288, 63,  918, 593, 888, 31,  483,
      413, 967, 277, 285, 709, 885, 382, 577, 858, 625, 754, 72,  437, 110,
      513, 410, 650, 428, 699, 170, 310, 147, 724, 898, 576, 951, 744, 877,
      741, 922, 882, 78,  553, 456, 34,  296, 981, 286, 381, 737, 458, 953,
      720, 125, 747, 9,   388, 675, 897, 468, 898, 149, 512, 352, 845, 247,
      127, 38,  6,   652, 448, 295, 859, 639, 236, 207, 931, 370, 868, 404,
      159, 716, 853, 265, 902, 239, 126, 900, 517, 622, 728, 565, 833, 88,
      832, 797, 920, 54,  452, 453, 980, 223, 947, 40,  339, 877, 411, 353,
      211, 855, 493, 178, 830, 828, 729, 989, 743, 451, 98,  325, 730, 733,
      560, 786, 879, 341, 570, 451, 316, 647, 203, 276, 812, 312, 907, 698,
      526, 174, 436, 573, 843, 820, 42,  764, 755, 903, 512, 444, 274, 638,
      943, 354, 45,  414, 864, 374, 220, 276, 805, 68,  772, 181, 23,  822,
      484, 12,  253, 779, 704, 314, 435, 144, 877, 516, 264, 716, 24,  910,
      800, 255, 598, 924, 517, 560, 761, 425, 8,   569, 84,  859, 715, 5,
      310, 391, 118, 790, 727, 509, 20,  941, 48,  63,  936, 89,  202, 442,
      472, 23,  921, 229, 16,  7,   170, 858, 643, 612, 701, 4,   926, 706,
      648, 399, 188, 498, 18,  897, 251, 292, 22,  396, 251, 901, 298, 937,
      969, 241, 553, 661, 774, 438, 101, 568, 133, 712, 138, 586, 319, 381,
      658, 403, 242, 723, 932, 905, 674, 291, 998, 184, 76,  569, 119, 283,
      278, 457, 62,  237, 565, 325, 732, 87,  204, 817, 820, 785, 120, 233,
      613, 363, 485, 801, 687, 31,  267, 513, 615, 386, 988, 362, 591, 893,
      403, 849, 914, 470, 861, 35,  931, 677, 475, 544, 107, 285, 815, 847,
      480, 845, 636, 618, 54,  815, 601, 635, 917, 60,  748, 110, 841, 792,
      641, 77,  400, 864, 226, 393, 799, 573, 783, 897, 932, 155, 226, 393,
      973, 110, 605, 387, 688, 509, 627, 778, 340, 948, 417, 20,  949, 479,
      639, 464, 835, 483, 234, 480, 616, 435, 963, 989, 463, 475, 294, 470,
      470, 419, 336, 223, 926, 733, 298, 833, 207, 392, 696, 1,   640, 378,
      384, 39,  706, 263, 886, 259, 714, 474, 551, 948, 199, 741, 169, 115,
      376, 317, 945, 161, 935, 943, 59,  125, 735, 125, 592, 882, 514, 130,
      428, 255, 472, 611, 106, 276, 420, 214, 29,  973, 422, 554, 106, 891,
      350, 423, 999, 832, 108, 932, 31,  154, 867, 251, 516, 226, 923, 369,
      293, 141, 752, 555, 988, 879, 872, 219, 457, 417, 739, 539, 115, 928,
      202, 318, 613, 421, 656, 233, 781, 535, 663, 538, 354, 17,  528, 259,
      546, 866, 711, 173, 628, 457, 522, 29,  744, 384, 249, 221, 625, 539,
      244, 577, 840, 591, 866, 904, 316, 733, 963, 4,   520, 557, 631, 749,
      505, 525, 747, 120, 267, 82,  11,  720, 945, 971, 955, 24,  868, 95,
      702, 611, 598, 229, 656, 497, 96,  649, 156, 401, 974, 316, 410, 78,
      767, 624, 86,  814, 843, 116, 821, 646, 487, 444, 527, 220, 543, 311,
      877, 887, 753, 603, 155, 300, 954, 855, 770, 774, 223, 665, 904, 747,
      316, 308, 562, 403, 523, 208, 765, 511, 368, 205, 321, 205, 710, 770,
      486, 171, 213, 122, 827, 426, 516, 357, 720, 433, 180, 164, 543, 545,
      280, 69,  128, 480, 390, 891, 860, 843, 346, 276, 630, 480, 317, 868,
      838, 595, 140, 435, 679, 553, 650, 831, 767, 298, 255, 334, 300, 438,
      190, 862, 533, 647, 5,   472, 129, 358, 895, 483, 158, 498, 417, 946,
      736, 855, 998, 209, 297, 590, 518, 828, 927, 30,  260, 6,   263, 503,
      692, 334, 688, 870, 589, 486, 985, 532, 1,   192, 345, 784, 660, 526,
      265, 887, 215, 421, 386, 192, 418, 868, 954, 299, 296, 534, 373, 429,
      513, 986, 106, 35,  227, 518, 558, 585, 239, 622, 685, 402, 154, 419,
      986, 829, 439, 980, 999, 681, 694, 807, 306, 452, 533, 843, 876, 799,
      187, 627, 962, 152, 825, 27,  461, 390, 818, 61,  704, 728, 364, 248,
      418, 584, 160, 784, 871, 409, 754, 625, 945, 358, 493, 353, 289, 283,
      382, 226, 250, 404, 250, 161, 421, 870, 891, 575, 565, 201, 885, 981,
      525, 146, 193, 85,  984, 932, 828, 735, 650, 140, 123, 429, 342, 195,
      910, 254, 726, 97,  277, 362, 487, 257, 238, 462, 442, 432, 854, 924,
      740, 284, 114, 170, 869, 353, 927, 483, 654, 765, 146, 165, 733, 979,
      492, 93,  490, 415, 695, 88,  670, 317, 954, 691, 320, 259, 378, 577,
      595, 493, 679, 688, 592, 594, 601, 821, 75,  913, 257, 316, 238, 803,
      194, 19,  796, 240, 278, 429, 830, 955, 879, 292, 512, 529, 248, 315,
      794, 185, 830, 985, 801, 456, 581, 803, 491, 207, 5,   895, 479, 124,
      728, 221, 382, 623, 116, 110, 188, 546, 543, 896, 219, 526, 415, 618,
      257, 898, 647, 52,  114, 880, 67,  943, 279, 925, 519, 224, 728, 372,
      312, 137, 160, 359, 706, 380, 813, 524, 33,  508, 287, 53,  880, 965,
      218, 372, 436, 565, 438, 501, 709, 196, 286, 809, 953, 176, 674, 922,
      799, 235, 757, 331, 727, 813, 412, 476, 302, 958, 86,  464, 965, 773,
      716, 986, 882, 198, 779, 286, 385, 400, 512, 219, 349, 139, 992, 793,
      691, 979, 914, 239, 885, 104, 801, 209, 36,  289, 656, 495, 747, 279,
      668, 957, 621, 80,  362, 420, 560, 89,  288, 677, 857, 129, 432, 348,
      586, 606, 655, 747, 963, 923, 43,  223, 139, 416, 376, 354, 528, 870,
      910, 979, 417, 909, 473, 258, 94,  391, 734, 350, 408, 573, 995, 340,
      959, 672, 75,  764, 824, 687, 66,  939, 982, 234, 190, 807, 550, 867,
      655, 536, 233, 390, 963, 621, 243, 682, 220, 887, 583, 642, 80,  185,
      860, 125, 246, 460, 799, 573, 848, 828, 235, 788, 333, 423, 263, 400,
      619, 36,  272, 602, 739, 331, 326, 320, 471, 996, 506, 486, 939, 512,
      346, 241, 517, 348, 747, 645, 222, 309, 829, 200, 605, 50,  463, 622,
      964, 593, 439, 204, 214, 207, 792, 128, 136, 173, 227, 700, 538, 201,
      641, 186, 478, 557, 162, 117, 412, 298, 606, 619, 997, 208, 631, 266,
      601, 78,  460, 10,  981, 103, 642, 48,  897, 311, 36,  621, 26,  611,
      41,  412, 68,  631, 953, 165, 839, 931, 846, 931, 81,  639, 130, 191,
      671, 126, 113, 273, 380, 463, 768, 628, 186, 168, 724, 68,  656, 251,
      72,  792, 406, 699, 806, 957, 845, 529, 811, 678, 698, 810, 533, 991,
      917, 709, 841, 817, 907, 874, 600, 290, 715, 378, 994, 376, 473, 206,
      487, 17,  323, 227, 23,  691, 606, 330, 729, 935, 987, 571, 750, 598,
      152, 227, 976, 915, 998, 719, 966, 369, 675, 533, 519, 701, 974, 571,
      51,  402, 230, 985, 617, 655, 681, 171, 83,  92,  627, 903, 524, 772,
      975, 572, 300, 806, 698, 352, 710, 742, 847, 263, 503, 4,   467, 600,
      156, 624, 9,   558, 328, 949, 387, 982, 200, 245, 925, 344, 299, 480,
      546, 400, 372, 531, 707, 882, 290, 914, 561, 101, 543, 214, 558, 34,
      855, 242, 639, 557, 178, 26,  301, 342, 784, 619, 471, 517, 980, 304,
      388, 175, 358, 978, 879, 250, 299, 840, 464, 371, 137, 567, 466, 692,
      104, 870, 93,  880, 567, 166, 620, 281, 581, 950, 131, 344, 867, 759,
      403, 697, 407, 518, 877, 311, 19,  331, 955, 633, 513, 896, 440, 591,
      652, 136, 529, 360, 488, 134, 289, 927, 953, 606, 943, 795, 912, 673,
      129, 438, 615, 384, 891, 319, 177, 266, 731, 197, 119, 83,  491, 423,
      874, 72,  505, 993, 562, 446, 917, 116, 84,  600, 489, 578, 208, 39,
      904, 856, 757, 202, 836, 356, 565, 199, 575, 676, 478, 769, 656, 792,
      161, 747, 189, 232, 930, 685, 846, 50,  510, 823, 109, 676, 862, 388,
      459, 516, 993, 71,  826, 980, 771, 665, 608, 734, 798, 939, 678, 394,
      759, 688, 893, 239, 503, 328, 324, 108, 51,  956, 101, 434, 814, 572,
      423, 100, 756, 675, 784, 584, 691, 135, 224, 171, 195, 877, 188, 615,
      458, 582, 948, 528, 676, 894, 810, 402, 421, 822, 51,  545, 935, 121,
      230, 117, 141, 43,  853, 812, 716, 219, 436, 945, 788, 541, 563, 279,
      49,  927, 987, 307, 915, 434, 126, 241, 170, 577, 79,  193, 779, 112,
      333, 511, 404, 98,  305, 721, 449, 432, 60,  113, 230, 986, 815, 751,
      146, 159, 253, 784, 896, 137, 878, 933, 394, 696, 68,  299, 868, 670,
      911, 180, 123, 557, 96,  48,  296, 62,  111, 329, 500, 601, 301, 722,
      596, 671, 361, 230, 437, 191, 794, 493, 919, 518, 220, 446, 985, 740,
      804, 481, 295, 651, 151, 559, 347, 31,  234, 871, 784, 175, 244, 524,
      984, 430, 502, 80,  373, 709, 226, 878, 40,  611, 306, 718, 697, 609,
      829, 580, 220, 524, 810, 657, 74,  804, 698, 293, 585, 237, 352, 966,
      208, 804, 779, 248, 461, 928, 316, 714, 59,  142, 696, 547, 490, 583,
      202, 812, 273, 898, 775, 834, 869, 163, 206, 33,  675, 889, 250, 164,
      752, 433, 782, 376, 433, 286, 917, 751, 180, 288, 87,  773, 506, 267,
      931, 690, 263, 410, 721, 384, 91,  443, 528, 462, 884, 810, 261, 268,
      609, 163, 393, 987, 950, 386, 516, 871, 9,   812, 153, 283, 142, 701,
      141, 341, 547, 263, 165, 306, 965, 858, 338, 703, 146, 682, 758, 14,
      599, 190, 64,  324, 308, 654, 366, 619, 158, 575, 995, 987, 471, 535,
      625, 68,  811, 799, 756, 757, 143, 801, 967, 344, 66,  425, 410, 948,
      898, 518, 928, 318, 347, 897, 468, 672, 957, 111, 930, 787, 810, 91,
      234, 1,   145, 718, 532, 84,  450, 571, 971, 25,  748, 164, 236, 489,
      559, 331, 217, 386, 402, 715, 52,  144, 602, 422, 237, 765, 190, 929,
      278, 882, 550, 980, 366, 644, 776, 66,  575, 380, 795, 141, 892, 927,
      476, 739, 346, 229, 984, 112, 282, 851, 898, 673, 631, 778, 196, 222,
      177, 886, 910, 868, 145, 116, 266, 986, 348, 345, 700, 952, 36,  118,
      798, 902, 171, 448, 499, 479, 385, 226, 533, 380, 723, 553, 198, 315,
      213, 497, 920, 905, 738, 215, 698, 121, 229, 294, 413, 189, 150, 168,
      891, 60,  871, 928, 760, 117, 560, 824, 629, 714, 964, 163, 32,  583,
      482, 562, 290, 459, 763, 901, 503, 818, 463, 528, 142, 356, 655, 733,
      123, 380, 960, 929, 223, 86,  178, 191, 45,  32,  513, 641, 166, 66,
      4,   571, 715, 824, 960, 864, 693, 294, 637, 473, 530, 999, 202, 146,
      880, 894, 666, 690, 59,  98,  387, 83,  319, 874, 556, 663, 332, 220,
      852, 643, 900, 925, 257, 21,  727, 9,   315, 193, 129, 458, 687, 866,
      825, 871, 932, 961, 485, 135, 946, 952, 524, 874, 496, 628, 602, 609,
      436, 799, 536, 185, 752, 611, 486, 487, 450, 266, 576, 108, 164, 736,
      664, 215, 2,   35,  891, 431, 123, 216, 815, 546, 941, 41,  806, 75,
      884, 367, 496, 962, 50,  38,  877, 932, 536, 86,  967, 623, 941, 571,
      677, 157, 418, 258, 420, 763, 168, 50,  159, 533, 654, 323, 801, 75,
      564, 328, 964, 106, 726, 60,  848, 902, 926, 556, 323, 43,  697, 618,
      931, 867, 741, 372, 714, 460, 358, 462, 616, 656, 112, 521, 876, 441,
      398, 15,  627, 791, 631, 458, 96,  261, 284, 814, 62,  675, 483, 479,
      87,  426, 483, 181, 378, 144, 563, 550, 366, 510, 350, 345, 649, 338,
      346, 639, 654, 188, 381, 955, 760, 983, 770, 549, 910, 834, 631, 836,
      208, 747, 226, 398, 739, 596, 496, 126, 308, 803, 43,  764, 128, 562,
      730, 586, 85,  511, 118, 983, 305, 168, 297, 64,  744, 548, 928, 516,
      208, 942, 801, 455, 490, 780, 686, 897, 575, 304, 98,  462, 248, 238,
      856, 115, 109, 124, 54,  826, 995, 66,  341, 837, 725, 706, 581, 633,
      84,  532, 229, 503, 836, 764, 535, 687, 368, 856, 828, 616, 898, 371,
      941, 100, 854, 603, 374, 622, 292, 345, 733, 447, 34,  352, 513, 921,
      197, 282, 762, 747, 139, 205, 492, 793, 376, 429, 949, 145, 220, 495,
      3,   582, 419, 305, 980, 302, 495, 833, 404, 332, 33,  390, 562, 234,
      85,  342, 524, 303, 279, 950, 127, 176, 676, 550, 196, 211, 715, 845,
      480, 772, 903, 376, 682, 632, 883, 177, 326, 990, 498, 316, 929, 511,
      559, 430, 758, 599, 403, 669, 723, 545, 612, 774, 891, 687, 391, 295,
      785, 809, 336, 768, 253, 642, 99,  215, 790, 470, 104, 919, 663, 847,
      62,  420, 142, 400, 5,   287, 510, 146, 149, 925, 798, 585, 133, 97,
      622, 347, 385, 713, 748, 668, 193, 882, 464, 855, 151, 825, 385, 450,
      464, 794, 446, 425, 533, 464, 966, 894, 828, 711, 826, 194, 237, 162,
      603, 377, 452, 306, 192, 733, 846, 125, 37,  801, 815, 904, 107, 393,
      19,  890, 512, 218, 168, 11,  592, 617, 622, 149, 71,  908, 204, 935,
      840, 123, 367, 97,  573, 633, 693, 866, 43,  849, 975, 463, 528, 161,
      285, 737, 407, 478, 867, 899, 376, 38,  609, 152, 575, 473, 501, 755,
      662, 832, 871, 340, 770, 270, 723, 640, 153, 376, 225, 836, 623, 659,
      342, 798, 523, 461, 454, 916, 736, 772, 617, 491, 834, 393, 496, 532,
      661, 90,  562, 663, 744, 115, 864, 175, 648, 629, 548, 758, 933, 369,
      777, 891, 46,  69,  601, 98,  333, 737, 860, 167, 395, 965, 846, 453,
      792, 743, 675, 490, 567, 908, 145, 803, 501, 282, 683, 308, 88,  583,
      420, 306, 730, 647, 437, 340, 335, 588, 356, 40,  867, 762, 78,  605,
      116, 485, 164, 800, 560, 115, 663, 280, 779, 606, 460, 769, 123, 896,
      91,  545, 793, 469, 734, 737, 134, 244, 532, 462, 32,  73,  882, 384,
      596, 933, 300, 799, 335, 876, 594, 43,  519, 528, 535, 84,  3,   776,
      235, 490, 864, 306, 746, 879, 364, 566, 581, 328, 870, 692, 362, 960,
      219, 264, 685, 715, 120, 531, 145, 403, 146, 528, 577, 103, 481, 567,
      310, 267, 529, 676, 542, 403, 84,  150, 619, 453, 743, 986, 619, 528,
      21,  155, 906, 835, 91,  559, 332, 458, 866, 545, 303, 929, 517, 755,
      844, 189, 259, 119, 423, 19,  79,  679, 742, 987, 575, 919, 139, 271,
      502, 879, 587, 862, 155, 222, 929, 408, 1,   230, 497, 5,   508, 206,
      234, 733, 845, 887, 463, 542, 183, 171, 966, 283, 994, 368, 795, 741,
      871, 578, 927, 921, 29,  36,  604, 107, 173, 723, 461, 507, 166, 236,
      160, 537, 664, 473, 227, 417, 363, 330, 184, 753, 147, 865, 890, 213,
      639, 386, 639, 172, 395, 313, 556, 948, 657, 240, 874, 925, 336, 474,
      783, 450, 810, 136, 478, 852, 975, 199, 753, 636, 235, 160, 404, 720,
      263, 976, 861, 818, 529, 689, 421, 794, 212, 810, 604, 690, 167, 809,
      371, 965, 129, 827, 895, 688, 692, 905, 546, 259, 634, 279, 843, 581,
      385, 990, 797, 648, 444, 962, 298, 71,  554, 434, 788, 40,  26,  618,
      212, 295, 961, 143, 820, 654, 182, 763, 414, 11,  468, 478, 141, 84,
      310, 824, 115, 623, 846, 360, 159, 128, 679, 181, 99,  104, 951, 686,
      288, 21,  394, 42,  847, 74,  457, 304, 888, 574, 312, 743, 485, 469,
      182, 297, 274, 457, 908, 666, 916, 627, 321, 694, 357, 586, 114, 918,
      630, 335, 339, 69,  473, 213, 176, 441, 251, 510, 884, 32,  281, 930,
      383, 476, 674, 550, 395, 727, 490, 458, 83,  235, 270, 957, 388, 783,
      368, 506, 175, 291, 165, 616, 363, 956, 94,  546, 173, 660, 83,  85,
      69,  591, 981, 58,  449, 12,  87,  484, 528, 229, 173, 490, 837, 670,
      419, 396, 506, 508, 108, 234, 655, 627, 207, 805, 280, 211, 506, 287,
      128, 800, 884, 510, 780, 569, 451, 437, 542, 306, 975, 273, 488, 830,
      221, 3,   688, 147, 443, 537, 489, 442, 750, 542, 401, 934, 587, 578,
      90,  108, 516, 929, 167, 214, 785, 790, 807, 993, 451, 716, 3,   259,
      46,  842, 838, 581, 60,  356, 753, 500, 379, 520, 764, 929, 205, 372,
      524, 542, 250, 39,  926, 154, 252, 506, 551, 849, 240, 239, 35,  312,
      154, 733, 179, 393, 683, 561, 882, 128, 644, 10,  546, 228, 163, 954,
      871, 1,   49,  123, 65,  94,  208, 928, 140, 907, 763, 379, 240, 184,
      334, 30,  532, 786, 377, 613, 326, 80,  85,  807, 237, 886, 430, 858,
      760, 547, 291, 362, 997, 448, 780, 939, 51,  615, 852, 363, 108, 324,
      536, 825, 253, 240, 155, 594, 773, 473, 643, 582, 292, 45,  403, 920,
      722, 526, 922, 710, 259, 438, 111, 925, 235, 24,  172, 761, 921, 741,
      40,  183, 171, 776, 505, 472, 630, 124, 395, 902, 968, 542, 971, 591,
      164, 105, 832, 932, 268, 681, 507, 521, 863, 354, 294, 802, 115, 742,
      769, 470, 10,  298, 958, 784, 742, 252, 200, 459, 849, 697, 14,  823,
      552, 311, 227, 611, 754, 893, 22,  232, 289, 78,  246, 63,  32,  478,
      45,  230, 821, 709, 456, 412, 983, 959, 971, 245, 248, 822, 117, 627,
      230, 690, 393, 719, 418, 102, 493, 22,  178, 510, 815, 694, 283, 555,
      242, 555, 429, 160, 546, 763, 124, 666, 460, 865, 376, 440, 173, 550,
      236, 801, 210, 714, 474, 971, 363, 650, 72,  786, 579, 934, 307, 615,
      12,  153, 965, 536, 608, 845, 155, 938, 771, 509, 747, 827, 297, 175,
      424, 604, 284, 265, 653, 851, 945, 173, 929, 779, 778, 204, 146, 630,
      609, 97,  773, 354, 628, 462, 676, 98,  208, 125, 531, 282, 417, 318,
      135, 42,  816, 658, 599, 918, 629, 428, 454, 800, 583, 55,  301, 806,
      283, 534, 619, 3,   800, 661, 962, 830, 607, 112, 496, 262, 110, 708,
      925, 529, 755, 795, 218, 234, 722, 556, 415, 468, 525, 574, 787, 381,
      634, 2,   904, 861, 641, 975, 685, 696, 189, 726, 535, 198, 879, 550,
      467, 661, 4,   594, 430, 637, 121, 469, 220, 912, 294, 696, 691, 23,
      558, 588, 90,  280, 123, 189, 468, 921, 923, 523, 593, 729, 362, 846,
      889, 77,  162, 685, 950, 913, 895, 924, 802, 935, 298, 359, 848, 377,
      910, 291, 630, 649, 900, 463, 702, 354, 750, 212, 748, 102, 308, 391,
      690, 970, 959, 729, 507, 100, 535, 731, 542, 681, 429, 449, 299, 897,
      130, 813, 485, 236, 799, 360, 281, 869, 270, 582, 136, 7,   196, 88,
      889, 133, 306, 696, 443, 356, 316, 902, 866, 649, 697, 442, 99,  246,
      360, 951, 422, 868, 939, 552, 12,  962, 583, 946, 669, 414, 575, 527,
      590, 753, 507, 123, 947, 623, 657, 608, 93,  44,  80,  454, 411, 946,
      360, 989, 873, 483, 403, 519, 918, 42,  134, 176, 193, 545, 346, 891,
      608, 249, 985, 294, 291, 878, 2,   302, 183, 325, 908, 246, 412, 993,
      336, 224, 185, 549, 663, 450, 539, 862, 333, 284, 811, 180, 694, 783,
      405, 241, 873, 309, 849, 609, 187, 719, 514, 559, 328, 183, 854, 351,
      469, 525, 625, 225, 728, 201, 766, 104, 513, 547, 345, 624, 929, 333,
      282, 853, 281, 126, 224, 797, 109, 775, 193, 352, 525, 448, 732, 976,
      397, 629, 640, 315, 299, 396, 196, 909, 22,  57,  884, 943, 615, 670,
      723, 990, 164, 685, 805, 502, 606, 845, 588, 641, 52,  423, 633, 939,
      293, 774, 907, 522, 998, 959, 497, 487, 966, 48,  913, 780, 768, 477,
      527, 277, 642, 192, 644, 160, 27,  613, 414, 923, 124, 880, 493, 754,
      301, 480, 720, 434, 803, 33,  699, 264, 183, 99,  64,  176, 453, 594,
      904, 762, 321, 243, 369, 892, 295, 226, 787, 142, 903, 680, 371, 725,
      666, 90,  171, 991, 127, 849, 297, 181, 373, 595, 108, 74,  531, 231,
      544, 25,  912, 328, 242, 584, 895, 207, 433, 957, 179, 728, 235, 249,
      6,   559, 300, 40,  749, 908, 790, 108, 700, 836, 731, 622, 914, 870,
      278, 688, 672, 505, 27,  438, 62,  552, 605, 74,  808, 683, 304, 386,
      29,  947, 574, 176, 621, 524, 573, 27,  403, 675, 812, 242, 622, 148,
      825, 310, 704, 834, 654, 812, 963, 601, 507, 825, 97,  840, 570, 8,
      596, 394, 287, 670, 614, 709, 997, 213, 728, 18,  848, 291, 263, 541,
      894, 241, 223, 268, 1,   779, 555, 88,  566, 527, 351, 764, 796, 385,
      725, 734, 163, 44,  85,  67,  800, 271, 989, 890, 649, 310, 709, 98,
      239, 116, 138, 57,  566, 326, 180, 110, 573, 774, 849, 16,  518, 757,
      333, 471, 298, 47,  41,  116, 402, 595, 554, 462, 875, 766, 971, 467,
      189, 5,   121, 250, 342, 885, 283, 94,  185, 825, 389, 679, 592, 429,
      862, 256, 132, 942, 505, 209, 981, 94,  265, 137, 346, 704, 33,  4,
      585, 53,  196, 560, 678, 204, 229, 249, 371, 960, 245, 782, 876, 455,
      634, 397, 311, 129, 935, 173, 886, 709, 439, 326, 546, 846, 411, 247,
      58,  416, 173, 914, 943, 419, 612, 16,  878, 237, 274, 847, 434, 376,
      538, 875, 109, 246, 101, 476, 887, 213, 92,  510, 486, 327, 388, 91,
      86,  340, 395, 259, 810, 513, 390, 760, 521, 466, 730, 294, 565, 496,
      474, 834, 794, 642, 841, 698, 766, 143, 606, 57,  586, 443, 258, 870,
      335, 65,  546, 463, 141, 966, 818, 213, 192, 445, 371, 896, 151, 706,
      961, 722, 531, 384, 48,  689, 10,  772, 210, 823, 726, 37,  422, 538,
      46,  479, 699, 181, 959, 929, 846, 658, 301, 17,  738, 777, 421, 259,
      209, 952, 938, 77,  886, 506, 791, 423, 203, 74,  18,  870, 506, 974,
      390, 797, 675, 278, 74,  809, 663, 432, 231, 907, 914, 587, 284, 375,
      920, 791, 553, 760, 595, 12,  86,  269, 317, 127, 547, 631, 382, 144,
      571, 799, 759, 331, 921, 188, 294, 618, 672, 238, 463, 848, 237, 520,
      83,  477, 362, 995, 729, 385, 909, 198, 247, 78,  881, 280, 394, 929,
      62,  150, 682, 156, 6,   182, 770, 764, 743, 660, 769, 611, 123, 592,
      231, 309, 741, 151, 418, 740, 763, 260, 122, 277, 198, 682, 497, 360,
      310, 981, 645, 844, 671, 249, 812, 507, 237, 224, 398, 360, 910, 22,
      913, 899, 988, 960, 494, 60,  636, 211, 304, 27,  112, 864, 825, 702,
      942, 905, 660, 387, 910, 575, 367, 652, 608, 841, 948, 820, 82,  243,
      592, 518, 977, 980, 57,  661, 957, 84,  284, 936, 270, 496, 343, 485,
      306, 891, 3,   742, 774, 697, 899, 596, 679, 562, 116, 449, 963, 363,
      886, 768, 225, 109, 809, 463, 166, 846, 515, 489, 813, 127, 42,  72,
      48,  602, 777, 760, 64,  634, 158, 356, 866, 37,  414, 444, 135, 141,
      29,  75,  810, 909, 779, 265, 27,  802, 461, 49,  235, 460, 114, 667,
      872, 346, 658, 496, 330, 533, 281, 52,  426, 577, 220, 946, 959, 135,
      341, 872, 606, 185, 788, 97,  972, 734, 153, 338, 637, 508, 979, 976,
      918, 708, 557, 593, 851, 21,  427, 546, 404, 483, 239, 785, 886, 365,
      262, 243, 834, 438, 119, 600, 158, 64,  451, 535, 106, 291, 579, 681,
      400, 583, 794, 184, 413, 47,  547, 441, 464, 228, 808, 884, 676, 682,
      675, 388, 518, 466, 554, 529, 106, 207, 76,  204, 774, 480, 778, 947,
      670, 528, 307, 362, 282, 498, 633, 890, 793, 754, 369, 770, 609, 306,
      816, 371, 169, 249, 284, 87,  633, 319, 79,  835, 876, 910, 884, 195,
      488, 110, 341, 732, 699, 154, 928, 253, 624, 550, 941, 530, 222, 492,
      996, 466, 439, 118, 563, 56,  216, 740, 200, 935, 74,  464, 334, 955,
      361, 527, 931, 191, 736, 118, 380, 159, 119, 760, 925, 736, 547, 182,
      577, 333, 900, 903, 828, 51,  686, 510, 813, 95,  233, 248, 593, 7,
      303, 418, 495, 13,  494, 870, 981, 520, 608, 631, 401, 938, 148, 910,
      480, 55,  325, 190, 485, 871, 257, 23,  46,  867, 716, 222, 182, 783,
      338, 642, 803, 492, 960, 988, 241, 591, 463, 604, 902, 266, 313, 805,
      844, 334, 848, 444, 651, 575, 513, 900, 211, 487, 377, 443, 47,  836,
      353, 528, 477, 920, 61,  288, 807, 623, 461, 47,  521, 469, 669, 176,
      697, 755, 622, 97,  991, 364, 554, 951, 938, 878, 194, 810, 807, 991,
      529, 172, 717, 489, 940, 850, 163, 130, 286, 316, 985, 367, 761, 726,
      347, 829, 351, 10,  135, 429, 348, 632, 789, 107, 82,  922, 452, 714,
      769, 418, 903, 637, 921, 720, 498, 722, 954, 167, 313, 640, 46,  795,
      862, 727, 517, 216, 425, 652, 448, 979, 345, 243, 625, 349, 995, 959,
      439, 264, 196, 131, 947, 398, 679, 682, 890, 330, 444, 315, 441, 471,
      83,  952, 930, 310, 588, 715, 824, 164, 241, 824, 118, 602, 712, 799,
      493, 787, 932, 732, 935, 215, 487, 747, 101, 500, 934, 182, 815, 998,
      463, 785, 237, 916, 880, 6,   767, 335, 539, 115, 122, 177, 131, 558,
      131, 394, 900, 912, 630, 422, 216, 634, 1,   215, 312, 599, 622, 511,
      447, 946, 580, 553, 120, 472, 176, 922, 471, 145, 596, 432, 110, 628,
      653, 898, 237, 745, 393, 410, 507, 264, 726, 944, 303, 410, 297, 689,
      772, 922, 513, 748, 248, 471, 359, 691, 278, 471, 258, 583, 701, 928,
      764, 72,  211, 625, 143, 54,  651, 909, 400, 475, 406, 713, 753, 320,
      260, 581, 561, 218, 957, 597, 399, 573, 421, 674, 839, 160, 542, 697,
      703, 347, 793, 76,  887, 427, 484, 931, 251, 797, 10,  445, 985, 454,
      473, 410, 904, 320, 29,  323, 675, 616, 748, 314, 717, 718, 869, 202,
      9,   141, 603, 628, 184, 686, 879, 912, 730, 285, 852, 55,  28,  582,
      828, 678, 62,  965, 686, 685, 253, 165, 558, 950, 962, 900, 841, 835,
      598, 322, 323, 25,  491, 273, 295, 58,  367, 479, 450, 603, 638, 47,
      416, 758, 908, 97,  310, 484, 443, 205, 198, 716, 916, 602, 439, 789,
      604, 697, 109, 934, 484, 61,  699, 393, 121, 843, 964, 377, 241, 672,
      927, 6,   645, 509, 218, 911, 490, 305, 189, 523, 314, 834, 566, 295,
      632, 67,  333, 254, 345, 468, 842, 580, 791, 142, 942, 683, 771, 863,
      339, 3,   697, 119, 672, 515, 153, 611, 438, 370, 59,  151, 419, 364,
      714, 367, 815, 731, 688, 102, 838, 393, 348, 573, 32,  165, 768, 702,
      986, 607, 997, 133, 726, 84,  35,  173, 666, 122, 893, 777, 183, 191,
      579, 566, 681, 150, 213, 263, 246, 910, 179, 626, 152, 781, 377, 253,
      270, 97,  875, 359, 486, 598, 856, 665, 743, 449, 787, 773, 810, 150,
      38,  329, 527, 801, 657, 970, 742, 703, 871, 363, 803, 87,  232, 778,
      707, 51,  624, 515, 937, 893, 679, 598, 367, 496, 858, 538, 597, 13,
      413, 317, 22,  215, 528, 398, 945, 786, 277, 882, 314, 361, 711, 647,
      223, 443, 358, 661, 805, 988, 566, 321, 754, 276, 146, 526, 234, 730,
      746, 755, 900, 641, 336, 751, 411, 692, 745, 536, 940, 364, 816, 268,
      317, 528, 543, 310, 467, 337, 175, 141, 283, 249, 185, 570, 193, 180,
      927, 729, 674, 697, 506, 353, 102, 892, 23,  164, 95,  757, 619, 762,
      278, 449, 148, 524, 444, 949, 3,   329, 581, 251, 405, 686, 180, 926,
      825, 148, 391, 515, 419, 674, 779, 842, 530, 592, 599, 61,  537, 973,
      388, 82,  1,   314, 967, 42,  39,  523, 557, 106, 984, 717, 678, 467,
      130, 859, 644, 852, 794, 411, 882, 137, 932, 862, 638, 998, 471, 560,
      758, 36,  976, 491, 753, 865, 524, 314, 565, 430, 282, 987, 406, 948,
      247, 410, 286, 981, 284, 569, 260, 486, 196, 444, 138, 61,  681, 72,
      428, 412, 514, 810, 15,  871, 898, 193, 208, 719, 680, 78,  613, 467,
      886, 272, 138, 740, 881, 730, 126, 328, 558, 596, 870, 738, 404, 691,
      907, 735, 275, 286, 386, 965, 212, 856, 871, 919, 167, 15,  132, 529,
      479, 499, 475, 602, 807, 626, 836, 133, 343, 458, 330, 446, 984, 859,
      822, 315, 11,  679, 714, 202, 108, 296, 808, 445, 741, 647, 20,  514,
      224, 806, 475, 922, 146, 132, 486, 344, 659, 24,  363, 24,  635, 55,
      548, 901, 159, 259, 664, 886, 954, 106, 673, 186, 491, 440, 947, 581,
      527, 939, 934, 196, 992, 625, 216, 55,  698, 498, 461, 371, 102, 368,
      979, 28,  554, 246, 136, 772, 483, 510, 787, 459, 267, 749, 325, 390,
      642, 508, 822, 666, 89,  861, 780, 741, 250, 660, 974, 875, 436, 432,
      444, 726, 570, 882, 395, 881, 334, 50,  386, 841, 937, 723, 779, 582,
      214, 168, 839, 425, 538, 327, 177, 832, 304, 104, 333, 677, 119, 155,
      423, 208, 520, 300, 764, 86,  256, 866, 749, 487, 237, 178, 22,  55,
      46,  115, 759, 721, 812, 284, 662, 446, 231, 600, 4,   686, 270, 46,
      177, 78,  556, 541, 856, 665, 342, 278, 811, 349, 180, 803, 728, 881,
      6,   880, 930, 896, 424, 216, 218, 519, 1,   632, 119, 162, 388, 492,
      209, 270, 689, 152, 327, 294, 6,   757, 402, 404, 753, 536, 538, 508,
      346, 165, 580, 269, 964, 20,  27,  224, 111, 432, 558, 144, 430, 157,
      976, 789, 967, 924, 426, 730, 428, 171, 500, 391, 42,  19,  697, 622,
      93,  465, 248, 360, 569, 570, 46,  794, 705, 206, 678, 696, 409, 391,
      617, 149, 911, 464, 228, 745, 812, 703, 514, 304, 511, 600, 389, 861,
      18,  711, 992, 763, 775, 397, 1,   581, 890, 86,  629, 470, 157, 407,
      661, 139, 613, 805, 166, 984, 758, 698, 260, 976, 216, 105, 917, 707,
      804, 952, 269, 536, 683, 641, 192, 530, 775, 929, 44,  924, 466, 140,
      175, 318, 60,  502, 793, 752, 684, 336, 140, 621, 754, 494, 290, 500,
      930, 385, 766, 305, 417, 282, 71,  397, 116, 691, 992, 313, 288, 795,
      329, 51,  981, 452, 655, 208, 651, 147, 354, 540, 390, 714, 911, 961,
      444, 904, 375, 683, 330, 988, 663, 809, 62,  273, 173, 885, 579, 469,
      794, 651, 308, 648, 250, 768, 662, 318, 971, 29,  442, 376, 679, 999,
      669, 605, 42,  760, 845, 519, 838, 341, 831, 824, 464, 216, 184, 833,
      448, 549, 840, 177, 481, 768, 695, 686, 129, 75,  540, 917, 737, 769,
      747, 888, 515, 309, 959, 256, 621, 740, 686, 669, 790, 672, 682, 895,
      180, 902, 840, 36,  385, 932, 867, 483, 493, 840, 82,  96,  272, 99,
      406, 255, 934, 997, 413, 369, 723, 311, 651, 74,  998, 728, 45,  932,
      225, 386, 792, 475, 968, 374, 563, 403, 458, 968, 834, 139, 197, 49,
      380, 866, 343, 919, 322, 503, 840, 884, 270, 398, 363, 825, 975, 404,
      721, 674, 409, 632, 534, 784, 774, 949, 88,  214, 243, 152, 779, 474,
      853, 641, 69,  393, 58,  831, 497, 904, 666, 582, 836, 809, 914, 465,
      603, 692, 655, 743, 390, 166, 495, 77,  636, 663, 179, 728, 407, 432,
      361, 381, 538, 101, 910, 314, 816, 341, 120, 742, 700, 666, 4,   297,
      157, 530, 277, 228, 609, 261, 327, 33,  189, 155, 78,  427, 978, 320,
      376, 414, 222, 361, 362, 708, 911, 435, 963, 406, 204, 176, 762, 890,
      239, 616, 698, 65,  435, 139, 416, 274, 344, 917, 182, 491, 163, 536,
      51,  90,  212, 522, 469, 51,  241, 36,  151, 287, 2,   612, 471, 674,
      781, 573, 142, 784, 447, 434, 467, 232, 10,  33,  295, 540, 502, 78,
      657, 914, 425, 457, 343, 827, 199, 894, 204, 44,  367, 452, 289, 394,
      558, 773, 807, 942, 662, 710, 671, 615, 737, 37,  508, 86,  602, 627,
      836, 649, 445, 211, 33,  158, 230, 441, 956, 627, 652, 807, 51,  583,
      789, 347, 456, 27,  465, 462, 919, 554, 916, 174, 216, 432, 865, 309,
      176, 715, 613, 561, 414, 906, 933, 952, 855, 163, 366, 482, 441, 295,
      329, 739, 330, 790, 179, 689, 987, 156, 628, 16,  841, 891, 483, 581,
      203, 968, 107, 39,  272, 50,  423, 783, 484, 147, 299, 476, 384, 770,
      178, 642, 723, 60,  608, 994, 869, 468, 864, 556, 606, 561, 735, 303,
      209, 938, 293, 234, 42,  813, 772, 792, 612, 639, 738, 420, 396, 189,
      127, 499, 438, 894, 537, 983, 263, 240, 5,   705, 393, 569, 865, 4,
      678, 1,   791, 396, 834, 985, 570, 507, 369, 902, 737, 569, 68,  889,
      119, 377, 995, 86,  447, 455, 92,  917, 174, 187, 925, 563, 596, 496,
      934, 248, 752, 900, 854, 933, 696, 983, 255, 52,  449, 775, 379, 680,
      693, 354, 387, 96,  637, 945, 275, 189, 454, 383, 63,  135, 480, 312,
      198, 130, 841, 523, 781, 732, 561, 957, 958, 644, 9,   521, 727, 607,
      665, 313, 240, 204, 108, 306, 338, 788, 720, 455, 362, 407, 939, 418,
      767, 392, 357, 705, 266, 373, 228, 368, 266, 926, 362, 686, 944, 281,
      582, 695, 146, 428, 901, 749, 671, 937, 963, 292, 893, 436, 897, 206,
      959, 433, 22,  478, 799, 507, 946, 22,  910, 476, 823, 555, 17,  426,
      734, 595, 947, 416, 821, 575, 871, 234, 988, 646, 707, 746, 91,  926,
      512, 439, 46,  714, 724, 89,  963, 284, 10,  698, 131, 38,  766, 416,
      191, 33,  595, 697, 248, 623, 224, 728, 807, 365, 32,  524, 943, 85,
      805, 208, 307, 734, 108, 573, 82,  833, 478, 192, 209, 831, 36,  494,
      11,  842, 484, 182, 93,  971, 205, 661, 514, 358, 661, 811, 381, 203,
      662, 974, 146, 434, 434, 521, 6,   8,   643, 637, 15,  98,  703, 524,
      388, 849, 392, 183, 627, 532, 560, 530, 867, 933, 111, 612, 559, 242,
      901, 390, 215, 967, 655, 323, 547, 581, 470, 69,  612, 43,  28,  65,
      526, 112, 89,  394, 909, 190, 63,  548, 799, 192, 874, 764, 446, 888,
      511, 427, 982, 910, 346, 893, 137, 547, 176, 469, 284, 993, 909, 88,
      383, 774, 469, 819, 222, 260, 315, 282, 23,  62,  893, 38,  369, 956,
      522, 366, 952, 92,  729, 731, 993, 955, 697, 391, 682, 235, 837, 556,
      154, 54,  809, 734, 958, 986, 929, 803, 677, 683, 838, 273, 486, 554,
      505, 610, 819, 227, 881, 546, 487, 614, 537, 50,  812, 730, 679, 543,
      595, 349, 74,  704, 306, 127, 944, 769, 910, 319, 299, 646, 129, 577,
      607, 822, 768, 680, 882, 23,  512, 262, 892, 701, 287, 983, 965, 838,
      993, 391, 736, 58,  560, 345, 803, 396, 60,  569, 193, 182, 370, 694,
      745, 92,  169, 930, 969, 979, 134, 327, 38,  237, 489, 941, 965, 815,
      386, 731, 371, 607, 327, 411, 296, 197, 379, 113, 966, 703, 648, 214,
      511, 796, 644, 383, 162, 919, 866, 528, 192, 349, 922, 908, 685, 671,
      955, 217, 294, 187, 963, 468, 617, 99,  854, 732, 513, 862, 792, 202,
      394, 402, 932, 870, 36,  669, 949, 554, 351, 343, 65,  906, 628, 927,
      887, 782, 804, 803, 4,   428, 970, 897, 537, 864, 918, 586, 16,  749,
      546, 709, 380, 557, 878, 158, 437, 999, 919, 210, 504, 535, 754, 520,
      737, 633, 993, 33,  365, 569, 955, 513, 712, 233, 943, 108, 725, 175,
      985, 287, 580, 377, 470, 184, 131, 695, 831, 543, 340, 803, 286, 150,
      227, 109, 403, 563, 380, 504, 911, 66,  403, 707, 297, 371, 441, 31,
      5,   858, 322, 176, 138, 618, 711, 678, 677, 949, 405, 244, 474, 783,
      748, 534, 361, 455, 665, 142, 520, 928, 422, 321, 477, 97,  882, 838,
      981, 891, 103, 604, 86,  484, 747, 760, 836, 330, 258, 452, 297, 613,
      357, 130, 605, 19,  828, 632, 306, 488, 805, 968, 597, 125, 50,  742,
      994, 534, 88,  29,  356, 927, 804, 598, 53,  272, 557, 725, 557, 415,
      944, 764, 700, 402, 735, 887, 802, 599, 263, 934, 735, 939, 305, 864,
      941, 838, 82,  47,  657, 559, 753, 866, 200, 903, 417, 389, 925, 395,
      621, 878, 670, 420, 835, 598, 590, 153, 162, 537, 700, 507, 247, 814,
      468, 290, 174, 209, 129, 855, 950, 922, 381, 759, 982, 32,  527, 452,
      63,  746, 504, 161, 625, 385, 266, 715, 442, 980, 771, 766, 777, 890,
      862, 30,  3,   640, 982, 125, 902, 319, 731, 245, 276, 832, 345, 253,
      747, 528, 980, 649, 457, 560, 168, 76,  774, 960, 81,  961, 181, 185,
      919, 983, 644, 519, 821, 126, 40,  944, 200, 541, 703, 880, 83,  429,
      435, 119, 133, 715, 858, 74,  196, 312, 414, 658, 38,  835, 867, 640,
      846, 173, 286, 177, 693, 731, 128, 236, 447, 837, 961, 787, 830, 141,
      425, 877, 628, 386, 147, 8,   628, 415, 160, 463, 200, 760, 779, 440,
      93,  610, 473, 887, 258, 117, 938, 759, 217, 587, 523, 682, 779, 377,
      627, 897, 859, 50,  321, 140, 652, 759, 495, 753, 391, 912, 898, 807,
      536, 822, 779, 445, 880, 944, 261, 569, 556, 953, 854, 206, 677, 79,
      718, 33,  219, 934, 890, 624, 471, 708, 613, 394, 332, 8,   39,  3,
      685, 821, 377, 133, 63,  821, 524, 371, 690, 177, 391, 574, 524, 234,
      498, 548, 658, 224, 706, 111, 418, 198, 690, 257, 105, 158, 485, 840,
      334, 574, 346, 75,  312, 964, 307, 229, 510, 368, 482, 396, 199, 72,
      477, 829, 911, 88,  269, 654, 967, 46,  827, 253, 8,   687, 299, 403,
      461, 483, 476, 21,  354, 682, 179, 686, 262, 739, 540, 696, 957, 964,
      995, 523, 221, 526, 929, 433, 191, 959, 547, 414, 620, 273, 191, 478,
      116, 699, 642, 853, 412, 656, 390, 180, 462, 332, 590, 968, 644, 16,
      744, 998, 494, 253, 885, 806, 270, 787, 537, 9,   448, 943, 565, 673,
      386, 700, 535, 883, 681, 299, 821, 625, 347, 315, 53,  173, 52,  692,
      399, 82,  618, 830, 700, 339, 938, 319, 715, 616, 751, 880, 197, 26,
      791, 986, 381, 100, 257, 331, 315, 421, 595, 375, 490, 538, 916, 426,
      112, 828, 228, 163, 508, 557, 602, 103, 85,  190, 574, 356, 324, 500,
      306, 174, 167, 54,  742, 650, 204, 470, 603, 371, 148, 917, 808, 805,
      877, 469, 648, 447, 536, 368, 851, 711, 900, 968, 39,  460, 98,  993,
      659, 588, 744, 63,  926, 88,  82,  639, 507, 103, 462, 772, 458, 287,
      434, 466, 453, 879, 527, 686, 86,  828, 292, 203, 630, 90,  557, 262,
      890, 874, 25,  221, 132, 671, 354, 4,   391, 696, 204, 640, 691, 503,
      880, 697, 50,  916, 397, 573, 613, 649, 619, 624, 710, 10,  127, 636,
      228, 743, 572, 334, 895, 570, 189, 897, 185, 110, 93,  926, 129, 676,
      171, 547, 903, 634, 661, 417, 493, 25,  359, 865, 217, 376, 656, 461,
      795, 995, 868, 789, 675, 360, 496, 324, 489, 482, 247, 501, 591, 600,
      159, 188, 233, 572, 948, 595, 69,  839, 274, 143, 449, 580, 725, 302,
      540, 351, 813, 160, 429, 424, 937, 606, 398, 885, 195, 641, 934, 597,
      946, 665, 632, 761, 544, 26,  541, 162, 243, 492, 787, 887, 927, 377,
      900, 276, 976, 508, 466, 309, 403, 930, 283, 656, 307, 773, 741, 984,
      47,  446, 612, 321, 632, 429, 734, 146, 147, 969, 918, 570, 689, 280,
      890, 542, 60,  666, 263, 767, 226, 22,  287, 967, 997, 272, 662, 941,
      132, 282, 198, 909, 318, 139, 727, 887, 516, 404, 28,  322, 770, 61,
      467, 698, 419, 290, 481, 817, 788, 237, 72,  880, 468, 133, 400, 331,
      857, 808, 101, 56,  837, 334, 2,   970, 238, 332, 250, 424, 432, 525,
      740, 302, 68,  368, 862, 831, 402, 263, 790, 293, 662, 335, 946, 987,
      366, 309, 647, 139, 746, 884, 608, 960, 106, 175, 564, 481, 549, 859,
      916, 18,  289, 636, 310, 72,  805, 142, 853, 810, 808, 245, 125, 555,
      48,  134, 872, 280, 271, 297, 673, 259, 182, 678, 693, 383, 896, 986,
      6,   251, 536, 866, 146, 991, 132, 183, 497, 722, 564, 861, 536, 943,
      198, 331, 768, 992, 294, 538, 833, 483, 923, 599, 247, 457, 188, 325,
      422, 358, 432, 78,  177, 803, 4,   704, 823, 817, 720, 604, 768, 754,
      244, 157, 229, 908, 843, 431, 750, 60,  792, 125, 94,  84,  369, 211,
      107, 608, 214, 475, 320, 954, 968, 260, 194, 452, 559, 447, 322, 18,
      349, 955, 379, 874, 856, 267, 969, 200, 106, 973, 561, 635, 140, 59,
      210, 637, 863, 712, 529, 573, 360, 615, 141, 885, 714, 378, 790, 789,
      118, 660, 732, 481, 206, 967, 356, 614, 759, 108, 395, 594, 895, 743,
      942, 226, 578, 676, 535, 547, 306, 955, 444, 590, 466, 258, 828, 165,
      376, 984, 526, 863, 528, 867, 610, 288, 277, 424, 927, 415, 280, 558,
      702, 214, 264, 168, 253, 163, 794, 229, 177, 472, 829, 342, 334, 316,
      27,  696, 772, 915, 184, 103, 448, 265, 729, 519, 367, 464, 681, 458,
      286, 212, 264, 877, 51,  875, 370, 609, 963, 663, 880, 719, 941, 356,
      360, 254, 845, 363, 902, 50,  359, 121, 329, 352, 566, 392, 573, 274,
      939, 966, 24,  198, 372, 43,  197, 845, 498, 612, 907, 815, 809, 606,
      722, 449, 535, 987, 744, 963, 220, 974, 717, 570, 450, 630, 205, 144,
      223, 817, 435, 566, 534, 964, 624, 261, 392, 861, 818, 63,  508, 67,
      482, 474, 507, 329, 492, 583, 901, 840, 279, 811, 546, 351, 794, 845,
      771, 587, 158, 113, 855, 759, 467, 19,  323, 484, 641, 668, 304, 963,
      813, 326, 541, 560, 606, 790, 640, 881, 521, 455, 880, 588, 498, 580,
      22,  546, 573, 689, 353, 361, 514, 112, 815, 592, 300, 38,  151, 734,
      612, 239, 978, 931, 992, 911, 562, 646, 432, 856, 437, 520, 762, 789,
      595, 94,  55,  826, 733, 491, 765, 480, 911, 914, 81,  676, 384, 354,
      952, 833, 434, 829, 228, 521, 311, 954, 143, 457, 164, 868, 607, 693,
      401, 137, 502, 148, 658, 625, 944, 396, 770, 563, 913, 123, 545, 362,
      443, 527, 479, 814, 489, 65,  513, 828, 217, 939, 452, 426, 900, 843,
      833, 855, 180, 826, 331, 148, 567, 360, 680, 101, 951, 175, 778, 676,
      712, 910, 253, 15,  481, 339, 235, 179, 186, 80,  819, 43,  239, 325,
      996, 695, 61,  895, 486, 883, 842, 829, 56,  684, 799, 599, 533, 565,
      547, 440, 456, 435, 308, 106, 845, 694, 779, 466, 918, 572, 75,  390,
      706, 804, 93,  681, 789, 597, 511, 13,  60,  529, 866, 834, 463, 580,
      303, 858, 700, 478, 681, 429, 136, 781, 140, 728, 777, 542, 298, 549,
      722, 562, 718, 621, 616, 273, 694, 257, 242, 708, 682, 611, 673, 636,
      201, 723, 763, 223, 352, 478, 19,  78,  306, 287, 440, 581, 660, 602,
      994, 900, 587, 280, 741, 428, 5,   398, 662, 372, 102, 796, 485, 340,
      386, 993, 833, 768, 80,  423, 197, 690, 952, 519, 646, 12,  722, 160,
      786, 112, 630, 227, 770, 967, 799, 829, 334, 711, 489, 681, 224, 321,
      5,   809, 245, 76,  595, 875, 297, 961, 811, 857, 334, 546, 950, 488,
      463, 961, 71,  791, 966, 448, 562, 389, 983, 237, 706, 618, 879, 621,
      560, 981, 854, 22,  805, 719, 1,   13,  189, 243, 480, 531, 764, 895,
      558, 376, 497, 810, 715, 827, 178, 349, 623, 25,  652, 959, 760, 834,
      468, 878, 814, 988, 455, 563, 310, 441, 491, 111, 39,  785, 24,  503,
      38,  554, 776, 152, 770, 310, 726, 868, 788, 130, 963, 11,  314, 322,
      624, 199, 486, 612, 937, 706, 103, 832, 522, 879, 860, 989, 7,   729,
      372, 546, 552, 199, 80,  72,  110, 208, 751, 205, 908, 957, 12,  286,
      851, 611, 734, 327, 401, 761, 859, 637, 945, 185, 290, 181, 848, 628,
      148, 339, 449, 318, 271, 65,  699, 568, 475, 332, 416, 217, 450, 430,
      227, 310, 270, 304, 423, 813, 317, 918, 486, 575, 962, 981, 848, 97,
      320, 402, 917, 849, 990, 241, 510, 978, 177, 798, 137, 702, 306, 483,
      77,  391, 146, 401, 243, 145, 934, 717, 892, 937, 956, 477, 145, 687,
      684, 865, 364, 412, 949, 154, 864, 401, 878, 491, 865, 841, 151, 384,
      963, 495, 226, 862, 742, 491, 577, 90,  570, 936, 868, 57,  567, 828,
      613, 328, 356, 346, 566, 882, 110, 356, 242, 301, 614, 639, 36,  585,
      455, 865, 318, 178, 787, 656, 928, 335, 7,   317, 994, 892, 443, 712,
      220, 964, 105, 628, 954, 168, 290, 272, 12,  590, 68,  141, 122, 775,
      879, 793, 818, 946, 535, 369, 558, 267, 492, 598, 248, 580, 411, 62,
      931, 372, 325, 716, 898, 676, 483, 116, 88,  477, 258, 458, 274, 60,
      431, 765, 803, 591, 406, 147, 785, 628, 243, 571, 92,  708, 568, 748,
      890, 279, 280, 52,  991, 459, 27,  103, 513, 25,  624, 860, 480, 994,
      577, 210, 834, 951, 227, 731, 108, 342, 716, 420, 100, 926, 528, 152,
      694, 443, 101, 571, 376, 757, 43,  623, 908, 992, 224, 759, 300, 771,
      517, 963, 562, 160, 89,  201, 954, 159, 480, 729, 898, 893, 490, 268,
      823, 909, 133, 808, 931, 490, 51,  787, 8,   779, 727, 824, 202, 806,
      88,  332, 584, 982, 451, 318, 958, 708, 492, 87,  285, 905, 727, 867,
      674, 705, 497, 883, 17,  481, 219, 960, 676, 21,  747, 612, 609, 361,
      783, 628, 122, 168, 142, 768, 19,  772, 677, 518, 603, 580, 112, 36,
      920, 687, 430, 263, 448, 649, 988, 825, 660, 546, 624, 430, 132, 207,
      805, 793, 963, 142, 852, 635, 680, 705, 733, 113, 775, 807, 580, 147,
      744, 795, 645, 556, 571, 858, 11,  578, 935, 770, 746, 82,  577, 404,
      996, 378, 770, 764, 332, 562, 961, 516, 231, 585, 295, 164, 974, 316,
      596, 708, 917, 891, 545, 671, 119, 808, 548, 313, 453, 568, 496, 19,
      123, 235, 564, 381, 295, 623, 211, 105, 802, 585, 997, 401, 416, 540,
      250, 983, 103, 385, 234, 741, 35,  533, 824, 967, 741, 354, 713, 116,
      278, 551, 588, 333, 252, 619, 253, 139, 645, 102, 507, 61,  452, 48,
      932, 832, 549, 825, 682, 662, 890, 556, 608, 283, 614, 463, 108, 970,
      817, 82,  341, 766, 891, 406, 578, 92,  508, 665, 297, 288, 664, 318,
      277, 776, 839, 806, 877, 396, 718, 193, 8,   628, 2,   732, 338, 901,
      221, 515, 466, 997, 158, 452, 652, 830, 529, 164, 43,  574, 128, 305,
      210, 655, 86,  410, 352, 428, 392, 395, 528, 957, 382, 951, 539, 116,
      122, 653, 545, 492, 503, 266, 345, 379, 811, 77,  11,  690, 335, 60,
      194, 529, 864, 276, 730, 238, 667, 839, 137, 568, 455, 535, 867, 100,
      526, 690, 486, 48,  15,  500, 128, 662, 733, 973, 879, 858, 262, 918,
      772, 494, 55,  91,  928, 957, 671, 157, 10,  444, 945, 43,  103, 986,
      510, 556, 702, 122, 604, 455, 183, 20,  424, 231, 558, 811, 829, 848,
      89,  479, 95,  575, 761, 313, 814, 421, 569, 361, 652, 297, 219, 853,
      87,  599, 310, 905, 373, 501, 111, 692, 633, 324, 657, 984, 867, 321,
      348, 309, 381, 625, 265, 987, 720, 793, 714, 603, 309, 47,  241, 244,
      567, 739, 918, 215, 111, 252, 758, 473, 168, 306, 514, 766, 398, 713,
      646, 643, 184, 892, 392, 56,  106, 371, 467, 615, 746, 545, 196, 856,
      250, 326, 659, 522, 983, 431, 59,  229, 295, 142, 485, 754, 448, 22,
      235, 373, 616, 684, 596, 551, 760, 695, 625, 271, 292, 647, 991, 192,
      116, 580, 534, 364, 814, 120, 951, 398, 901, 639, 416, 643, 548, 392,
      138, 705, 539, 205, 416, 742, 177, 861, 177, 894, 637, 525, 296, 315,
      530, 509, 968, 142, 104, 743, 107, 845, 891, 243, 215, 851, 700, 217,
      128, 539, 2,   695, 732, 728, 724, 57,  252, 157, 854, 595, 313, 575,
      432, 604, 949, 465, 618, 40,  112, 619, 766, 236, 518, 304, 140, 113,
      405, 569, 963, 920, 746, 836, 338, 728, 711, 337, 534, 60,  37,  904,
      581, 982, 298, 745, 874, 667, 348, 519, 859, 144, 975, 31,  755, 901,
      909, 782, 401, 320, 83,  698, 237, 109, 905, 626, 775, 923, 66,  977,
      606, 828, 845, 748, 176, 331, 28,  342, 492, 962, 700, 691, 165, 603,
      177, 817, 727, 977, 673, 905, 32,  908, 548, 953, 841, 184, 313, 805,
      766, 491, 455, 865, 421, 861, 650, 674, 996, 943, 678, 748, 248, 402,
      581, 736, 514, 617, 837, 132, 258, 558, 188, 892, 468, 549, 500, 226,
      617, 564, 353, 357, 464, 573, 281, 171, 989, 719, 3,   543, 889, 716,
      564, 530, 853, 292, 73,  817, 235, 726, 36,  276, 260, 426, 742, 830,
      351, 139, 127, 626, 867, 664, 947, 951, 316, 214, 369, 326, 749, 904,
      225, 867, 2,   825, 33,  782, 189, 743, 219, 228, 254, 215, 133, 350,
      695, 329, 623, 834, 419, 874, 173, 347, 836, 946, 836, 494, 22,  642,
      170, 289, 209, 603, 891, 477, 194, 996, 797, 1,   116, 382, 376, 1,
      331, 79,  1,   668, 256, 64,  313, 990, 844, 799, 741, 804, 64,  842,
      951, 642, 638, 574, 109, 866, 445, 119, 565, 750, 430, 961, 624, 19,
      44,  408, 419, 94,  50,  924, 169, 771, 42,  831, 636, 365, 696, 744,
      165, 94,  327, 846, 897, 618, 864, 316, 503, 469, 748, 338, 345, 752,
      701, 758, 279, 668, 684, 309, 551, 458, 116, 696, 791, 91,  264, 3,
      734, 963, 994, 371, 17,  741, 788, 777, 939, 366, 570, 810, 290, 61,
      222, 384, 916, 664, 337, 483, 724, 307, 184, 971, 963, 701, 230, 688,
      262, 365, 242, 520, 53,  81,  623, 578, 80,  269, 202, 55,  376, 429,
      628, 510, 530, 140, 582, 488, 221, 836, 38,  920, 411, 338, 698, 561,
      919, 161, 371, 468, 899, 138, 280, 904, 732, 373, 196, 802, 789, 628,
      320, 463, 729, 675, 315, 185, 510, 360, 968, 673, 70,  122, 591, 702,
      832, 567, 359, 87,  440, 259, 207, 337, 54,  599, 277, 281, 917, 546,
      166, 537, 5,   226, 174, 181, 71,  640, 477, 636, 651, 368, 84,  753,
      217, 539, 75,  612, 721, 658, 194, 289, 606, 699, 993, 579, 78,  961,
      755, 468, 465, 308, 234, 424, 910, 355, 549, 357, 314, 255, 359, 752,
      547, 114, 578, 648, 565, 474, 561, 715, 889, 485, 21,  55,  366, 576,
      805, 466, 856, 404, 235, 844, 641, 329, 831, 806, 463, 199, 334, 975,
      78,  995, 359, 423, 402, 649, 262, 793, 26,  569, 422, 966, 919, 524,
      236, 586, 264, 284, 932, 962, 776, 560, 24,  302, 568, 674, 948, 619,
      887, 670, 460, 969, 599, 87,  532, 519, 738, 894, 931, 77,  593, 290,
      771, 357, 681, 850, 722, 933, 143, 237, 594, 195, 824, 910, 224, 485,
      204, 374, 553, 60,  232, 18,  238, 52,  751, 405, 535, 431, 122, 942,
      348, 257, 903, 20,  689, 18,  671, 21,  498, 9,   393, 113, 383, 198,
      672, 255, 827, 982, 274, 983, 294, 514, 423, 610, 272, 358, 788, 210,
      951, 722, 769, 20,  921, 372, 298, 388, 100, 350, 742, 668, 339, 201,
      167, 65,  263, 762, 484, 645, 6,   393, 903, 261, 95,  551, 416, 733,
      215, 998, 439, 394, 403, 32,  453, 735, 564, 998, 503, 424, 359, 441,
      291, 657, 115, 775, 996, 688, 377, 899, 849, 79,  277, 559, 537, 295,
      52,  519, 500, 312, 735, 898, 555, 141, 483, 133, 401, 183, 287, 998,
      929, 852, 986, 321, 685, 788, 171, 655, 323, 39,  355, 397, 519, 920,
      729, 132, 690, 155, 151, 298, 253, 169, 55,  412, 562, 661, 686, 381,
      839, 856, 620, 694, 39,  802, 967, 750, 427, 93,  803, 504, 883, 253,
      3,   540, 483, 303, 167, 366, 478, 76,  670, 427, 528, 969, 967, 417,
      117, 921, 782, 178, 874, 824, 289, 846, 277, 104, 63,  196, 686, 508,
      812, 831, 16,  337, 168, 530, 422, 819, 594, 156, 323, 326, 142, 304,
      767, 731, 87,  884, 946, 16,  502, 329, 240, 121, 114, 476, 176, 323,
      743, 782, 204, 371, 23,  918, 534, 55,  82,  192, 361, 842, 655, 330,
      319, 415, 501, 722, 622, 362, 226, 280, 590, 606, 732, 502, 800, 847,
      16,  772, 349, 346, 816, 424, 244, 602, 3,   431, 942, 426, 447, 21,
      566, 127, 490, 509, 390, 980, 206, 635, 331, 669, 614, 414, 619, 170,
      100, 932, 667, 939, 687, 995, 784, 492, 505, 936, 723, 215, 844, 527,
      603, 548, 946, 378, 919, 951, 980, 832, 925, 269, 510, 163, 850, 913,
      384, 29,  614, 150, 953, 38,  676, 736, 853, 93,  421, 418, 664, 134,
      228, 866, 35,  668, 732, 858, 682, 80,  891, 467, 131, 758, 489, 442,
      490, 735, 257, 426, 904, 977, 37,  108, 223, 563, 491, 150, 566, 283,
      209, 30,  169, 725, 846, 361, 789, 269, 235, 870, 629, 6,   727, 4,
      360, 795, 678, 683, 586, 573, 768, 273, 328, 375, 786, 804, 186, 450,
      351, 160, 945, 252, 473, 219, 693, 7,   752, 727, 424, 931, 182, 994,
      817, 714, 113, 482, 720, 988, 813, 85,  306, 477, 160, 340, 221, 894,
      576, 87,  237, 886, 759, 659, 26,  301, 144, 342, 297, 910, 866, 544,
      22,  949, 108, 494, 168, 187, 988, 941, 539, 972, 501, 761, 69,  344,
      805, 88,  903, 924, 581, 657, 228, 959, 565, 721, 959, 170, 321, 564,
      697, 678, 219, 727, 506, 344, 535, 225, 271, 608, 928, 347, 367, 33,
      352, 319, 657, 175, 592, 827, 57,  545, 13,  443, 557, 745, 970, 396,
      402, 392, 341, 137, 392, 404, 225, 970, 464, 464, 78,  848, 535, 174,
      750, 735, 149, 918, 947, 653, 658, 971, 413, 416, 596, 191, 341, 961,
      48,  958, 288, 412, 194, 68,  653, 609, 128, 638, 714, 8,   503, 225,
      260, 572, 411, 595, 333, 215, 847, 194, 843, 77,  72,  346, 792, 243,
      694, 434, 852, 135, 322, 548, 510, 512, 943, 740, 789, 651, 803, 404,
      951, 3,   33,  813, 714, 565, 764, 810, 158, 515, 458, 161, 868, 719,
      879, 557, 931, 422, 149, 277, 786, 440, 308, 9,   460, 954, 836, 281,
      853, 211, 959, 179, 115, 555, 821, 712, 965, 172, 652, 853, 1,   33,
      771, 28,  760, 133, 738, 67,  792, 925, 337, 440, 296, 339, 607, 880,
      387, 748, 746, 329, 653, 499, 447, 629, 434, 608, 606, 574, 445, 68,
      661, 685, 688, 172, 115, 754, 406, 82,  464, 512, 259, 520, 142, 455,
      943, 421, 68,  858, 595, 43,  354, 263, 403, 945, 299, 980, 53,  29,
      793, 339, 473, 257, 546, 747, 863, 608, 675, 223, 160, 464, 799, 271,
      68,  941, 176, 606, 124, 910, 465, 390, 418, 660, 581, 274, 525, 966,
      19,  215, 490, 977, 983, 84,  609, 87,  275, 403, 529, 258, 79,  433,
      472, 491, 753, 537, 823, 243, 446, 842, 232, 651, 214, 483, 156, 746,
      940, 671, 409, 640, 575, 888, 488, 462, 995, 140, 372, 631, 279, 982,
      3,   414, 933, 583, 77,  179, 794, 360, 670, 859, 554, 69,  821, 508,
      36,  518, 982, 820, 604, 672, 562, 138, 51,  631, 701, 361, 510, 880,
      230, 652, 549, 975, 907, 196, 666, 25,  908, 621, 690, 85,  470, 553,
      555, 914, 142, 299, 536, 165, 429, 42,  871, 973, 222, 228, 100, 346,
      340, 398, 328, 36,  100, 646, 484, 843, 532, 240, 325, 55,  371, 975,
      440, 942, 145, 631, 739, 23,  543, 419, 804, 244, 563, 441, 123, 335,
      454, 169, 295, 582, 433, 200, 858, 945, 769, 782, 639, 10,  92,  799,
      358, 566, 400, 898, 202, 28,  483, 845, 487, 105, 399, 148, 167, 147,
      885, 955, 117, 433, 563, 849, 632, 698, 700, 424, 47,  524, 625, 407,
      103, 470, 660, 57,  487, 893, 46,  142, 260, 396, 932, 993, 97,  199,
      156, 631, 706, 697, 53,  843, 513, 549, 645, 77,  255, 461, 763, 835,
      610, 711, 460, 577, 988, 6,   822, 753, 252, 149, 643, 559, 937, 367,
      307, 905, 272, 929, 887, 186, 197, 638, 245, 508, 605, 509, 936, 59,
      82,  186, 722, 864, 591, 938, 103, 811, 95,  792, 36,  531, 4,   233,
      411, 337, 68,  814, 144, 704, 830, 125, 190, 725, 724, 857, 938, 79,
      67,  491, 965, 672, 598, 308, 205, 957, 152, 877, 788, 937, 415, 860,
      489, 406, 265, 486, 784, 628, 719, 944, 305, 979, 469, 621, 726, 322,
      411, 833, 92,  417, 88,  41,  224, 38,  291, 487, 100, 137, 782, 251,
      459, 572, 605, 791, 740, 918, 926, 344, 330, 536, 728, 712, 406, 523,
      989, 934, 131, 859, 370, 278, 444, 320, 747, 697, 307, 564, 945, 557,
      777, 71,  708, 73,  61,  565, 64,  722, 721, 549, 24,  506, 497, 125,
      769, 705, 967, 26,  40,  596, 851, 679, 354, 715, 768, 576, 634, 180,
      431, 543, 553, 374, 822, 784, 580, 706, 543, 494, 828, 515, 921, 98,
      431, 688, 758, 658, 394, 264, 673, 132, 882, 870, 903, 799, 434, 578,
      431, 836, 684, 526, 57,  695, 423, 289, 191, 79,  478, 201, 750, 352,
      538, 524, 374, 444, 640, 203, 11,  261, 739, 274, 736, 903, 554, 529,
      583, 318, 805, 120, 83,  454, 211, 809, 705, 797, 78,  287, 388, 79,
      950, 744, 304, 788, 734, 830, 425, 247, 873, 916, 346, 908, 255, 578,
      410, 236, 202, 490, 40,  93,  898, 690, 921, 684, 662, 177, 885, 380,
      838, 774, 529, 112, 444, 922, 990, 683, 150, 191, 61,  794, 390, 274,
      510, 477, 120, 998, 366, 318, 897, 361, 786, 864, 480, 104, 206, 603,
      10,  410, 27,  447, 726, 715, 682, 711, 736, 351, 576, 670, 903, 896,
      237, 537, 802, 939, 215, 977, 443, 785, 193, 271, 913, 795, 234, 54,
      108, 325, 463, 111, 248, 943, 359, 450, 677, 815, 810, 967, 764, 181,
      769, 251, 706, 624, 416, 378, 287, 542, 176, 695, 860, 200, 235, 523,
      560, 401, 687, 532, 375, 980, 630, 266, 57,  895, 172, 888, 296, 286,
      507, 212, 439, 193, 512, 442, 272, 556, 914, 853, 916, 902, 388, 347,
      866, 778, 883, 821, 200, 569, 620, 237, 399, 3,   516, 780, 994, 148,
      838, 69,  830, 57,  152, 996, 534, 846, 861, 444, 126, 812, 733, 565,
      930, 767, 943, 79,  453, 108, 632, 714, 679, 249, 219, 344, 127, 514,
      821, 319, 363, 959, 731, 262, 53,  1,   536, 3,   624, 400, 800, 305,
      462, 487, 228, 794, 404, 949, 794, 33,  507, 233, 230, 394, 276, 459,
      591, 918, 241, 162, 700, 941, 488, 792, 546, 987, 115, 333, 589, 536,
      315, 738, 657, 823, 761, 436, 926, 676, 319, 101, 474, 622, 8,   639,
      830, 705, 91,  353, 938, 687, 595, 242, 397, 866, 302, 11,  436, 748,
      479, 918, 699, 771, 327, 685, 11,  898, 229, 314, 988, 68,  349, 974,
      368, 73,  648, 167, 942, 154, 479, 51,  418, 717, 104, 239, 419, 693,
      692, 642, 491, 478, 913, 764, 995, 3,   767, 530, 579, 127, 827, 780,
      459, 934, 995, 284, 784, 515, 788, 374, 357, 795, 176, 226, 735, 787,
      537, 257, 44,  61,  943, 955, 569, 190, 728, 330, 846, 147, 545, 376,
      441, 899, 158, 828, 10,  215, 123, 987, 478, 798, 507, 383, 232, 522,
      834, 448, 386, 696, 328, 371, 578, 212, 20,  165, 49,  868, 729, 164,
      911, 208, 251, 203, 505, 197, 866, 698, 891, 747, 138, 250, 516, 754,
      181, 448, 170, 75,  460, 311, 512, 479, 429, 585, 580, 356, 706, 801,
      724, 305, 261, 368, 566, 994, 16,  286, 737, 627, 127, 133, 169, 51,
      78,  373, 814, 733, 207, 449, 516, 304, 594, 853, 621, 335, 829, 104,
      638, 187, 579, 848, 62,  712, 960, 240, 188, 544, 891, 537, 650, 848,
      242, 244, 793, 289, 577, 771, 37,  388, 645, 732, 651, 760, 417, 68,
      376, 279, 243, 338, 371, 586, 366, 369, 530, 267, 793, 115, 545, 959,
      946, 449, 789, 383, 167, 743, 314, 961, 196, 26,  113, 290, 405, 516,
      799, 408, 611, 634, 498, 434, 915, 490, 397, 82,  533, 676, 53,  236,
      257, 474, 747, 463, 460, 638, 504, 470, 679, 787, 175, 124, 600, 894,
      465, 1,   333, 41,  839, 596, 255, 367, 264, 878, 91,  646, 359, 736,
      943, 845, 938, 89,  269, 135, 274, 330, 319, 525, 150, 114, 675, 213,
      514, 265, 330, 599, 12,  463, 905, 741, 34,  898, 736, 53,  385, 978,
      688, 732, 524, 154, 921, 969, 743, 122, 817, 604, 554, 752, 11,  731,
      983, 497, 974, 795, 367, 395, 969, 774, 404, 864, 702, 349, 922, 990,
      863, 191, 791, 759, 890, 804, 185, 539, 669, 416, 776, 412, 506, 444,
      803, 653, 120, 580, 389, 825, 309, 828, 340, 912, 767, 273, 424, 220,
      397, 789, 588, 546, 525, 944, 396, 266, 64,  254, 569, 26,  613, 851,
      313, 632, 75,  10,  894, 239, 477, 75,  780, 855, 401, 951, 932, 191,
      73,  496, 687, 996, 706, 706, 514, 28,  465, 557, 904, 387, 676, 524,
      994, 712, 624, 989, 855, 784, 857, 839, 93,  660, 698, 731, 709, 764,
      430, 780, 226, 829, 76,  465, 820, 323, 722, 909, 339, 583, 419, 188,
      43,  398, 517, 50,  407, 577, 681, 191, 370, 832, 590, 16,  374, 891,
      328, 700, 652, 116, 380, 402, 349, 676, 205, 35,  548, 323, 247, 639,
      713, 268, 694, 825, 945, 89,  398, 874, 837, 822, 492, 665, 959, 790,
      486, 824, 587, 553, 289, 878, 522, 140, 904, 580, 670, 846, 393, 731,
      408, 439, 701, 113, 667, 738, 538, 904, 335, 700, 820, 28,  908, 542,
      219, 627, 470, 111, 962, 475, 301, 536, 539, 885, 919, 779, 898, 283,
      473, 542, 491, 528, 135, 55,  739, 729, 125, 64,  283, 789, 208, 472,
      752, 679, 719, 692, 775, 262, 100, 304, 80,  612, 806, 236, 904, 779,
      659, 745, 654, 492, 733, 556, 266, 944, 726, 865, 407, 33,  296, 314,
      951, 981, 196, 761, 559, 715, 676, 40,  309, 99,  449, 395, 291, 398,
      520, 560, 583, 594, 73,  351, 665, 805, 761, 335, 139, 501, 537, 666,
      616, 220, 990, 28,  400, 92,  162, 101, 457, 948, 414, 849, 501, 785,
      975, 663, 557, 237, 973, 364, 244, 844, 546, 589, 954, 872, 25,  62,
      153, 112, 696, 255, 961, 692, 214, 762, 749, 761, 894, 246, 935, 916,
      320, 281, 867, 432, 575, 53,  493, 148, 876, 163, 345, 25,  482, 104,
      571, 449, 598, 548, 387, 904, 798, 202, 869, 817, 824, 307, 791, 841,
      299, 491, 456, 228, 188, 582, 731, 67,  692, 3,   656, 22,  808, 100,
      528, 683, 817, 781, 308, 554, 362, 954, 697, 816, 143, 712, 755, 846,
      466, 739, 793, 936, 327, 614, 172, 832, 329, 577, 449, 759, 121, 467,
      783, 101, 678, 857, 577, 753, 431, 170, 295, 99,  481, 664, 216, 306,
      514, 545, 219, 192, 659, 781, 147, 995, 316, 361, 851, 306, 558, 742,
      156, 294, 253, 154, 809, 952, 571, 243, 193, 358, 447, 93,  293, 690,
      384, 343, 602, 100, 804, 116, 674, 266, 460, 424, 62,  926, 141, 478,
      5,   582, 183, 508, 132, 647, 728, 623, 475, 970, 811, 413, 420, 827,
      766, 843, 924, 943, 114, 822, 937, 674, 760, 739, 653, 258, 657, 280,
      903, 787, 94,  613, 13,  573, 832, 489, 129, 796, 423, 292, 572, 390,
      867, 404, 121, 318, 21,  851, 458, 601, 616, 487, 538, 836, 613, 90,
      126, 718, 122, 557, 828, 539, 679, 643, 637, 754, 539, 341, 968, 145,
      514, 587, 277, 692, 854, 336, 445, 377, 501, 602, 589, 812, 873, 869,
      62,  113, 448, 641, 400, 449, 412, 882, 69,  517, 897, 105, 937, 788,
      623, 731, 935, 144, 685, 762, 616, 537, 435, 567, 721, 346, 348, 173,
      30,  557, 39,  544, 631, 963, 593, 146, 493, 365, 246, 695, 816, 118,
      545, 355, 158, 949, 572, 94,  365, 974, 537, 876, 755, 111, 810, 199,
      685, 789, 68,  201, 145, 853, 345, 638, 791, 726, 211, 410, 729, 738,
      614, 349, 878, 193, 225, 125, 257, 667, 280, 502, 106, 31,  557, 533,
      285, 52,  564, 213, 312, 780, 349, 122, 273, 4,   364, 273, 223, 512,
      259, 420, 694, 933, 540, 821, 500, 937, 853, 484, 124, 914, 757, 640,
      161, 96,  572, 330, 35,  940, 703, 494, 143, 297, 413, 8,   326, 498,
      665, 94,  382, 441, 689, 737, 938, 602, 34,  849, 681, 897, 389, 275,
      723, 361, 176, 701, 702, 381, 267, 383, 347, 565, 928, 2,   186, 757,
      183, 845, 752, 790, 37,  649, 560, 984, 837, 135, 403, 4,   381, 145,
      414, 884, 755, 469, 433, 909, 632, 248, 190, 509, 869, 23,  188, 233,
      619, 220, 605, 486, 418, 205, 387, 528, 170, 347, 793, 534, 770, 105,
      112, 157, 536, 546, 928, 697, 607, 656, 795, 688, 243, 305, 424, 61,
      540, 950, 560, 998, 411, 845, 479, 722, 107, 269, 456, 233, 55,  326,
      124, 1,   658, 450, 426, 971, 128, 161, 262, 454, 744, 213, 195, 729,
      480, 211, 66,  465, 666, 523, 603, 223, 23,  486, 484, 970, 459, 387,
      891, 195, 912, 324, 840, 715, 6,   96,  510, 201, 79,  205, 332, 479,
      294, 696, 285, 812, 759, 787, 634, 791, 645, 870, 29,  334, 481, 779,
      410, 286, 380, 513, 221, 122, 206, 469, 788, 549, 10,  725, 264, 302,
      582, 371, 638, 71,  986, 502, 439, 331, 576, 571, 332, 981, 562, 665,
      874, 651, 688, 187, 558, 678, 838, 790, 888, 778, 536, 497, 115, 709,
      871, 641, 432, 751, 198, 248, 155, 149, 133, 963, 96,  637, 287, 359,
      80,  755, 802, 249, 391, 572, 305, 423, 451, 407, 302, 791, 741, 714,
      781, 877, 17,  294, 251, 59,  668, 429, 920, 13,  465, 934, 622, 134,
      755, 109, 205, 351, 158, 318, 800, 903, 666, 274, 642, 819, 930, 344,
      94,  550, 90,  798, 747, 85,  71,  177, 791, 745, 781, 199, 862, 411,
      601, 846, 368, 147, 962, 67,  607, 458, 33,  630, 42,  110, 935, 140,
      35,  144, 411, 294, 255, 94,  66,  496, 60,  278, 844, 98,  915, 63,
      102, 880, 369, 865, 784, 817, 263, 970, 460, 402, 299, 556, 790, 709,
      546, 718, 501, 342, 28,  210, 56,  21,  375, 298, 128, 400, 523, 811,
      377, 231, 814, 122, 358, 597, 141, 595, 643, 603, 863, 696, 243, 644,
      916, 988, 841, 374, 282, 940, 722, 780, 807, 260, 119, 605, 55,  303,
      263, 13,  16,  897, 10,  71,  33,  566, 872, 965, 627, 434, 807, 12,
      63,  497, 145, 849, 338, 448, 98,  529, 164, 482, 785, 617, 264, 521,
      789, 681, 907, 135, 681, 420, 219, 400, 652, 198, 324, 455, 124, 730,
      716, 555, 623, 934, 61,  21,  853, 13,  838, 168, 252, 624, 151, 223,
      889, 952, 880, 895, 445, 215, 482, 816, 944, 869, 593, 524, 585, 377,
      614, 125, 568, 906, 360, 729, 998, 817, 495, 641, 565, 405, 338, 759,
      469, 323, 286, 113, 508, 318, 465, 790, 908, 804, 241, 911, 520, 822,
      42,  159, 86,  635, 103, 57,  856, 965, 584, 563, 512, 991, 210, 786,
      147, 356, 169, 50,  406, 885, 232, 156, 778, 366, 491, 86,  185, 946,
      464, 793, 27,  213, 406, 210, 261, 548, 129, 924, 433, 983, 658, 421,
      878, 436, 434, 984, 516, 811, 106, 35,  235, 323, 911, 975, 186, 830,
      401, 695, 248, 855, 762, 202, 853, 94,  935, 86,  63,  298, 776, 934,
      410, 336, 107, 558, 562, 670, 378, 376, 712, 214, 233, 148, 172, 349,
      278, 884, 384, 112, 811, 859, 468, 145, 464, 278, 635, 734, 814, 651,
      724, 28,  691, 906, 61,  914, 118, 379, 440, 808, 728, 87,  110, 530,
      826, 3,   564, 391, 242, 4,   706, 678, 600, 343, 596, 951, 302, 888,
      11,  824, 686, 428, 252, 660, 682, 82,  8,   729, 745, 540, 633, 510,
      959, 532, 643, 801, 803, 617, 251, 316, 797, 48,  792, 908, 966, 986,
      933, 68,  199, 825, 196, 570, 415, 471, 382, 155, 470, 91,  932, 763,
      698, 174, 286, 199, 597, 394, 867, 34,  228, 871, 116, 262, 25,  833,
      141, 666, 430, 219, 480, 886, 918, 86,  701, 712, 576, 659, 633, 637,
      5,   320, 463, 945, 133, 27,  937, 166, 477, 333, 918, 439, 966, 17,
      308, 299, 136, 239, 304, 300, 763, 658, 500, 944, 255, 709, 624, 642,
      505, 393, 450, 920, 316, 926, 486, 516, 6,   400, 526, 834, 783, 386,
      504, 760, 493, 719, 321, 184, 195, 135, 151, 31,  55,  41,  521, 606,
      919, 874, 62,  131, 888, 540, 525, 728, 845, 955, 425, 703, 914, 452,
      213, 766, 930, 393, 766, 905, 227, 859, 845, 402, 780, 205, 469, 115,
      109, 128, 261, 395, 591, 657, 864, 929, 437, 149, 447, 608, 893, 639,
      12,  435, 453, 645, 113, 234, 87,  189, 890, 110, 66,  827, 533, 501,
      641, 31,  279, 773, 524, 267, 519, 396, 816, 719, 589, 87,  967, 867,
      722, 792, 399, 992, 395, 589};

  tuple* tuples1 = new tuple[10000];
  tuple* tuples2 = new tuple[15000];
  for (int64_t i = 0; i < 10000; i++) {
    tuples1[i] = {keys1[i], keys1[i]};
  }
  for (int64_t i = 0; i < 15000; i++) {
    tuples2[i] = {keys2[i], keys2[i]};
  }

  relation r(tuples1, 10000);
  relation s(tuples2, 15000);

  result t1 = PartitionedHashJoin_ST(r, s);
  TEST_CHECK(t1.getSize() == 149814);

  result t2 = PartitionedHashJoin_ST(s, r);
  TEST_CHECK(t2.getSize() == t1.getSize());
}

void test_svector() {
  simple_vector<int> test_v, test_v2;

  test_v.add_back(3);
  test_v.add_back(8);
  test_v.add_back(9);

  test_v2 = test_v;

  TEST_CHECK(test_v2[0] == 3);
  TEST_CHECK(test_v2[1] == 8);
  TEST_CHECK(test_v2[2] == 9);
  TEST_CHECK(test_v2.getSize() == 3);

  simple_vector<int> test_v3{test_v2};

  TEST_CHECK(test_v3.getSize() == 3);

  for (size_t i = 0; i < test_v3.getSize(); i++) {
    TEST_CHECK(test_v3[i] == test_v2[i]);
  }

  struct a {
    int x, y;
  };

  simple_vector<a> test_v4(1);
  test_v4.add_back({1, 2});
  test_v4.add_back({2, 3});
  test_v4.add_back({3, 4});
  test_v4.add_back({4, 5});
  test_v4.add_back({5, 6});

  TEST_CHECK(test_v4[0].x == 1);
  TEST_CHECK(test_v4[2].y == 4);
  TEST_CHECK(test_v4[4].x == 5);
  TEST_CHECK(test_v4.getSize() == 5);
}

void test_linked_list() {
  List l;
  tuple s[] = {{1, 2}, {1, 4}, {1, 5}, {3, 5}};
  for (size_t i = 0; i < 4; i++) l.append(&s[i]);
  TEST_CHECK(l.getLen() == 4);
  TEST_CHECK(l.find(s[0]));
  TEST_CHECK(l.find(s[1]));
  TEST_CHECK(l.find(s[2]));
  TEST_CHECK(l.find(s[3]));
}

void test_squeue() {
  simple_queue<int> sq;
  sq.enqueue(3);
  sq.enqueue(5);
  sq.enqueue(11);
  sq.enqueue(10);
  TEST_CHECK(sq.getLen() == 4);
  TEST_CHECK(sq.pop() == 3);
  TEST_CHECK(sq.pop() == 5);
  TEST_CHECK(sq.pop() == 11);
  TEST_CHECK(sq.pop() == 10);
  TEST_CHECK(sq.getLen() == 0);
}

TEST_LIST = {{"Partitioning function", test_partitioning_function},
             {"Partitioning - small test (One pass)", test_partitions_1},
             {"Partitioning - small test 2 (Two Pass)", test_partitions_2},
             {"Partitioning - big test (One Pass)", test_partitions_3},
             {"Partitioning - big test (Two Pass)", test_partitions_4},
             {"Test No Partitioning", test_no_partition},
             {"HT's Hash Function", test_HThash2},
             {"Normal HT Insert", test_HTinsert1},
             {"Same Key HT Insert", test_HTinsert2},
             {"Empty NBHD Slot HT Insert", test_HTinsert3},
             {"Full HT Insert", test_HTinsert4},
             {"Full NBHD HT Insert", test_HTinsert5},
             {"Swap HT Insert", test_HTinsert6},
             {"None for Swap HT Insert", test_HTinsert7},
             {"Relations equal amount of partitions", test_eq_partitions},
             {"Partition's HT has entry", test_build_1},
             {"Join with Partitioning (One Pass)", test_join_1},
             {"Join with Partitioning (Two Pass)", test_join_2},
             {"Join without Partitioning", test_join_3},
             {"Join Big", test_join_4},
             {"Join Huge", test_join_5},
             {"Test Simple Vector", test_svector},
             {"Test List", test_linked_list},
             {"Test Simple Queue,", test_squeue},
             {NULL, NULL}};
