#ifndef HT_H
#define HT_H
#include "dataForm.h"

class bitmap
{
    private:
    bool arr[H];

    public:
    bitmap() {}
    ~bitmap() {}
};

class bucket
{
    private:
    uint32_t val;
    bitmap *Bitmap;

    public:
    bucket() { this->Bitmap = new bitmap(); }
    ~bucket() { delete Bitmap; }
};

class hashTable
{
    private:
    bucket *buckets;
    uint32_t num_buckets;

    public:
    hashTable(uint32_t num_buckets)
    {
        this->num_buckets = num_buckets;
        this->buckets = new bucket[this->num_buckets];
    }
    ~hashTable() { delete buckets; }

    void insert(Histogram, uint32_t);
};
#endif