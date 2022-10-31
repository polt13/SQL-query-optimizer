#include "hashTable.h"

// for abs
#include <cmath>

List &bucket::getTuples() { return this->tuples; }

bool bucket::getOccupied() const { return this->occupied; }

bool bucket::getBitmapIndex(uint64_t index) const {
  return this->Bitmap[index];
}

void bucket::setTuple(tuple *t) { this->tuples.append(t); }

void bucket::setTuple(List &l) { this->tuples = l; }

void bucket::setOccupied(bool flag) { this->occupied = flag; }

void bucket::setBitmapIndex(uint64_t index, bool flag) {
  this->Bitmap[index] = flag;
}

bucket::bucket() : occupied{false}, Bitmap{} {}

// ------------------------------------------------------------------

int64_t hashTable::getBucketCount() const { return num_buckets; }

int64_t hashTable::hash2(int64_t key) { return key % this->num_buckets; }

void hashTable::rehash() {
  std::printf("Rehashing");
  int64_t old_bucket_count = num_buckets;
  bucket *old_buckets = buckets;

  num_buckets = old_bucket_count * 2 + 1;
  buckets = new bucket[num_buckets];

  for (int64_t i = 0; i < old_bucket_count; i++) {
    Node *traverse = old_buckets[i].getTuples().getRoot();
    while (traverse) {
      insert(traverse->mytuple);
      traverse = traverse->next;
    }
  }

  delete[] old_buckets;
}

// Insert all tuples of a partition into the hashTable
void hashTable::insert(tuple *t) {
  int64_t hashVal = hash2(t->getKey());
  bool flag;

  // ----- Implement Hopscotch Hashing -----
  if (this->buckets[hashVal].getOccupied() == false) {
    this->buckets[hashVal].setTuple(t);
    this->buckets[hashVal].setOccupied(true);
    this->buckets[hashVal].setBitmapIndex(0, true);
  } else {
    // Check if Neighbourhood is FULL and Check for Duplicates
    flag = true;  // Assume it's full
    for (int64_t i = 0; i < NBHD_SIZE; i++) {
      if (this->buckets[hashVal].getBitmapIndex(i) == false)
        flag = false;  // Neighbourhood NOT full - Empty slot FOUND
      else {
        // Check if exact same key (R.a) exists
        if (this->buckets[hashVal + i]
                .getTuples()
                .getRoot()
                ->mytuple->getKey() == t->getKey()) {
          // Add argument tuple into the List tuples of bucket
          this->buckets[hashVal + i].getTuples().append(t);
          return;
        }
      }
    }
    // Step 1. FULL Neighbourhood
    if (flag == true) {
      rehash();
      insert(t);
      return;
    }
    // Step 2.
    int64_t j = (hashVal + 1) % this->num_buckets;
    flag = false;
    while (j != hashVal) {
      if (this->buckets[j].getOccupied() == false) {  // Empty slot found
        flag = true;
        break;
      }
      j = (j + 1) % this->num_buckets;
    }

    if (flag == false) {
      // No empty slot - Rehash needed
      rehash();
      insert(t);
    }
    // Step 3.
    while ((std::abs((int64_t)(j - hashVal)) % this->num_buckets) >=
           NBHD_SIZE) {
      // Step 3.a.
      int64_t k = j - NBHD_SIZE + 1;
      // In case where the index turns out negative, cycle back to the end
      if (k < 0) k = num_buckets + k;
      // Search NBHD_SIZE - 1
      for (uint64_t x = 0; x < NBHD_SIZE - 1; x++) {
        if (this->buckets[k].getBitmapIndex(x) == true) {
          flag = true;  // Element found

          // Step 3.c
          this->buckets[j].setTuple(
              this->buckets[(k + x) % num_buckets].getTuples());
          this->buckets[j].setOccupied(true);
          this->buckets[k].setBitmapIndex((NBHD_SIZE - 1), true);

          List newlist;
          this->buckets[(k + x) % num_buckets].setTuple(newlist);
          this->buckets[(k + x) % num_buckets].setOccupied(false);
          this->buckets[k].setBitmapIndex(x, false);
          // Step 3.d.
          j = (k + x) % this->num_buckets;
          break;
        }
      }
      // Step 3.b. | No element found
      if (flag == false) {
        // Rehash needed
        rehash();
        insert(t);
        return;
      }
    }
    // Step 4 | Save tuple here
    this->buckets[j].setTuple(t);
    this->buckets[j].setOccupied(true);
    this->buckets[hashVal].setBitmapIndex(std::abs((int64_t)(j - hashVal)),
                                          true);
  }
}

void hashTable::findEntry(int64_t key) {
  int64_t hashVal = hash2(key);  // not sure if Key

  if (this->buckets[hashVal].getTuples().getRoot()->mytuple->getKey() == key)
    std::printf("Found item with key %ld\n", key);

  else {
    // Search inside neighbourhood
    for (uint64_t i = 0; i < NBHD_SIZE; i++) {
      if (this->buckets[hashVal + i].getTuples().getRoot()->mytuple->getKey() ==
          key) {
        std::printf("Found item with key %ld\n", key);
        return;
      }
    }
    std::printf("Item with key %ld NOT FOUND\n", key);
  }
}

hashTable::hashTable(int64_t num_tuples)
    : num_buckets{num_tuples}, buckets{new bucket[num_tuples]} {}

hashTable::~hashTable() { delete[] buckets; }