#ifndef HT_H
#define HT_H
#include "dataForm.h"
#include "partition.h"

#define H 4  // whatever

class bucket {
 private:
  uint64_t val;
  bool flag;
  bucket *Bitmap[H];

 public:
  // Getters
  uint64_t getVal() const;
  bool getFlag() const;
  bucket *getBitmapBucket(uint64_t);
  // Setters
  void setVal(uint64_t);
  void setFlag(bool);
  void setBitmapBucket(uint64_t, bucket *);

  bucket();
  ~bucket() { delete Bitmap; }
};

class hashTable {
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