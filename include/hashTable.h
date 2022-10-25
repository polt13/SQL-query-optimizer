#ifndef HT_H
#define HT_H
#include "dataForm.h"
#include "partition.h"

#define HBHD_SIZE 32  // Neigbourhood size

class bucket {
 private:
  tuple *mytuple;
  bool flag;
  bool Bitmap[HBHD_SIZE];
  //bucket *Bitmap[HBHD_SIZE];

 public:
  // Getters
  tuple *getTuple() const;
  bool getFlag() const;
  bool getBitmapIndex(uint64_t) const;
  //bucket *getBitmapBucket(uint64_t);
  // Setters
  void setTuple(tuple *);
  void setFlag(bool);
  void setBitmapIndex(uint64_t, bool);
  //void setBitmapBucket(uint64_t, bucket *);

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