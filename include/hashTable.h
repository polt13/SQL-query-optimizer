#ifndef HT_H
#define HT_H
#include "dataForm.h"
#include "partition.h"

#define H 4 // whatever

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
    uint64_t val;
    bitmap *Bitmap;

    public:
    // Getters
    uint64_t getVal() const;
    // Setters
    void setVal(uint64_t);
    
    bucket() { this->Bitmap = new bitmap(); }
    ~bucket() { delete Bitmap; }
};

class hashTable
{
    private:
    bucket *buckets;
    uint64_t num_buckets;

    public:
    // Getters
    uint64_t getBucketCount() const;

    uint64_t hash2(uint64_t);

    void insert(const tuple &);

    void fillHT(const Partition &);

    hashTable(uint64_t num_tuples);

    ~hashTable() { delete buckets; }
};
#endif