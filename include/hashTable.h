#ifndef HT_H
#define HT_H
#include "list.h"
#include "config.h"

class bucket {
  List tuples;
  bool occupied;
  bool Bitmap[NBHD_SIZE];

 public:
  // Getters
  List &getTuples();
  bool getOccupied() const;
  bool getBitmapIndex(int64_t) const;
  //  Setters
  void setTuple(tuple *);
  void setTuple(List &);
  void setOccupied(bool);
  void setBitmapIndex(int64_t, bool);

  bucket();
};

class hashTable {
  bucket *buckets;
  int64_t num_buckets;  // Basically HT Size
  char pad[48];

 public:
  // Getters
  bucket *getBucket(int64_t) const;
  int64_t getBucketCount() const;

  int64_t hash2(int64_t);
  void insert(tuple *);
  void rehash();
  List *findEntry(int64_t);

  hashTable(int64_t);
  ~hashTable();
};
#endif