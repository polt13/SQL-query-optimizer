
#ifndef CONFIG_H
#define CONFIG_H

constexpr int THREAD_COUNT = 4;

constexpr int USE_BITS = 8;  // first partitioning

constexpr int USE_BITS_NEXT = 10;  // 2nd partitioning

constexpr int L2_SIZE = 1024 * 1024 * 3;  // set to 256KB

constexpr int NBHD_SIZE = 32;  // Neigbourhood size

constexpr int UPPER_LIMIT = 50000000;  // 50,000,000

#endif