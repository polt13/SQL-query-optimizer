
#ifndef CONFIG_H
#define CONFIG_H
#include <cstdint>

constexpr int32_t THREAD_COUNT = 6;

constexpr int32_t THREAD_COUNT_QUERIES = 6;

constexpr int32_t USE_BITS = 4;  // first partitioning

constexpr int32_t USE_BITS_NEXT = 8;  // 2nd partitioning

constexpr int32_t L2_SIZE = 1024 * 1024 * 3;  // set to 256KB

constexpr int32_t NBHD_SIZE = 32;  // Neigbourhood size

constexpr int32_t UPPER_LIMIT = 50000000;  // 50,000,000

#endif