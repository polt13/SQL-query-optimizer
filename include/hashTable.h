#ifndef HT_H
#define HT_H
#include "list.h"
#define NBHD_SIZE 32  // Neigbourhood size

class bucket {
  List tuples;
  bool occupied;
  bool Bitmap[NBHD_SIZE];

 public:
  // Getters
  List &getTuples();
  bool getOccupied() const;
  bool getBitmapIndex(uint64_t) const;
  //  Setters
  void setTuple(tuple *);
  void setTuple(List &);
  void setOccupied(bool);
  void setBitmapIndex(uint64_t, bool);

  bucket();
};

class hashTable {
  bucket *buckets;
  int64_t num_buckets;  // Basically HT Size

 public:
  // Getters
  int64_t getBucketCount() const;

  int64_t hash2(int64_t);
  void insert(tuple *);
  void rehash();

  void findEntry(int64_t);

  hashTable(int64_t);
  ~hashTable();
};
#endif