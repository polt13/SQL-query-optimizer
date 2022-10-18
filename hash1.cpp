#include "interface.h"

/*
 * Hash Function for partitioning
 * Get the n Least Significant Bits (LSB)
 */
int32_t hash1(int32_t key, uint32_t n)
{
    uint32_t num = 1;
    num <<= n + 1;
    // 1000 - 1 = 111
    // val = key & (2^n - 1); // bitwise AND
    return key & (num - 1);
}