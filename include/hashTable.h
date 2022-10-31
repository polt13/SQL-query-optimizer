#ifndef HT_H
#define HT_H
#include "dataForm.h"
#define NBHD_SIZE 32  // Neigbourhood size

class bucket {
 private:
  tuple *mytuple;
  bool occupied;
  bool Bitmap[NBHD_SIZE];
  // bucket *Bitmap[HBHD_SIZE];

 public:
  // Getters
  tuple *getTuple() const;
  bool getOccupied() const;
  bool getBitmapIndex(int64_t) const;
  // bucket *getBitmapBucket(int64_t);
  //  Setters
  void setTuple(tuple *);
  void setOccupied(bool);
  void setBitmapIndex(int64_t, bool);
  // void setBitmapBucket(int64_t, bucket *);

  bucket();
  ~bucket();
};

class hashTable {
 private:
  bucket *buckets;
  int64_t num_buckets;  // Basically HT Size

 public:
  // Getters
  int64_t getBucketCount() const;

  int64_t hash2(int64_t);

  void insert(tuple *);

  void rehash();

  // void fillHT(const HistEntry &);

  void findEntry(int64_t);

  hashTable(int64_t);
  ~hashTable();
};
#endif