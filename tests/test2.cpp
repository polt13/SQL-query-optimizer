#include <cstdio>
#include "dataForm.h"
#include "map_info.h"
#include "query_exec.h"
#include "simple_vector.h"
#include "acutest.h"

memory_map rel_mmap[14];

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

TEST_LIST = {{"Test Simple Vector", test_svector}, NULL};