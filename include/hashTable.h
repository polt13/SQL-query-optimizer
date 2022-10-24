#ifndef HT_H
#define HT_H
#include "dataForm.h"
#include "partition.h"

#define H 4  // whatever

class bucket {
 private:
  tuple *mytuple;
  bool flag;
  bucket *Bitmap[H];

 public:
  // Getters
  tuple *getTuple() const;
  bool getFlag() const;
  bucket *getBitmapBucket(uint64_t);
  // Setters
  void setTuple(tuple *);
  void setFlag(bool);
  void setBitmapBucket(uint64_t, bucket *);

  bucket();
  ~bucket();
};

class hashTable {
 private:
  bucket *buckets;
  uint64_t num_buckets;

 public:
  // Getters
  uint64_t getBucketCount() const;

  uint64_t hash2(int64_t);

  void insert(tuple *);

  void fillHT(const Partition &);

  void findEntry(int64_t);

  hashTable(uint64_t);
  ~hashTable();
};
#endif