#include <interface.h>

/*
 * Hash Function for partitioning
 * Get the n Least Significant Bits (LSB)
 */
int32_t hash1(int32_t key, uint32_t n)
{
    int32_t val = 0;
    uint32_t num = 1, i;
    for (i = 0; i <= n; i++)
        num *= 2;
    // 1000 - 1 = 111
    // val = key & (2^n - 1); // bitwise AND
    val = key & (num - 1);
    return val;
}