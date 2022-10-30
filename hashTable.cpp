#include "hashTable.h"

#include <cmath>
#include <cstdio>

#include "list.h"

tuple *bucket::getTuple() const { return this->mytuple; }

bool bucket::getOccupied() const { return this->occupied; }

bool bucket::getBitmapIndex(uint64_t index) const {
  return this->Bitmap[index];
}

void bucket::setTuple(tuple *t) { this->mytuple = t; }

void bucket::setOccupied(bool flag) { this->occupied = flag; }

void bucket::setBitmapIndex(uint64_t index, bool flag) {
  this->Bitmap[index] = flag;
}

bucket::bucket() : mytuple{nullptr}, occupied{false}, Bitmap{} {}

bucket::~bucket() {}

// ------------------------------------------------------------------

int64_t hashTable::getBucketCount() const { return num_buckets; }

int64_t hashTable::hash2(int64_t key) { return key % this->num_buckets; }

void hashTable::rehash() {
  std::printf("Rehashing");
  int64_t old_bucket_count = num_buckets;
  bucket *old_buckets = buckets;

  num_buckets = old_bucket_count * 2 + 1;
  buckets = new bucket[num_buckets];

  for (int64_t i = 0; i < old_bucket_count; i++)
    insert(old_buckets[i].getTuple());

  delete[] old_buckets;
}

// Insert all tuples of a partition into the hashTable
void hashTable::insert(tuple *t) {
  uint64_t hashVal = hash2(t->getKey());
  bool flag;

  // ----- Implement Hopscotch Hashing -----
  if (this->buckets[hashVal].getOccupied() == false) {
    this->buckets[hashVal].setTuple(t);
    this->buckets[hashVal].setOccupied(true);
    this->buckets[hashVal].setBitmapIndex(0, true);
  } else {
    // Check if Neighbourhood is FULL and Check for Duplicates
    flag = true;  // Assume it's full
    for (uint64_t i = 0; i < NBHD_SIZE; i++) {
      if (this->buckets[hashVal].getBitmapIndex(i) == false)
        flag = false;  // Neighbourhood NOT full - Empty slot FOUND
      else {
        // Check if exact same key (R.a) exists
        if (this->buckets[hashVal + i].getTuple()->getKey() == t->getKey())
          // Add argument's rowID into the List payload
          this->buckets[hashVal + i].getTuple()->getPayload().append(
              t->getPayload().getRoot()->rowID);
      }
    }
    // Step 1. FULL Neighbourhood
    if (flag == true) {
      rehash();
      insert(t);
      return;
    }
    // Step 2.
    uint64_t j = (hashVal + 1) % this->num_buckets;
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
      // in case where the index turns out negative, cycle back to the end
      if (k < 0) k = num_buckets + k;
      // Search NBHD_SIZE - 1
      for (uint64_t x = 0; x < NBHD_SIZE - 1; x++) {
        if (this->buckets[k].getBitmapIndex(x) == true) {
          flag = true;  // Element found
          this->buckets[j].setTuple(
              this->buckets[(k + x) % num_buckets].getTuple());
          this->buckets[j].setOccupied(true);
          this->buckets[k].setBitmapIndex((NBHD_SIZE - 1), true);
          // Step 3.c
          this->buckets[(k + x) % num_buckets].setTuple(nullptr);
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

void hashTable::fillHT(const HistEntry &entry) {
  Node *traverse = entry.getHistEntries();
  while (traverse) {
    insert(&(traverse->t));
    traverse = traverse->next;
  }
}

void hashTable::findEntry(int64_t key) {
  uint64_t hashVal = hash2(key);  // not sure if Key

  if (this->buckets[hashVal].getTuple()->getKey() == key)
    std::printf("Found item with key %ld\n", key);

  else {
    // Search inside neighbourhood
    for (uint64_t i = 0; i < NBHD_SIZE; i++) {
      if (this->buckets[hashVal + i].getTuple()->getKey() == key) {
        std::printf("Found item with key %ld\n", key);
        return;
      }
    }
    std::printf("Item with key %ld NOT FOUND\n", key);
  }
}

hashTable::hashTable(int64_t num_tuples) : num_buckets{num_tuples} {
  if (num_tuples > 0) {
    /*
    this->num_buckets = 1;
    // Calculate number of buckets needed based on number of tuples
    // Must be power of 2 | e.g 1,2,4,8,...
    if (num_tuples > 1) {
      uint64_t exponent =
          (num_tuples / 2) - 1;  // -1 because num_buckets = 1 = 2^0
      this->num_buckets <<= exponent;
      // Buckets must be more than or equal to tuples, NOT LESS
      while (this->num_buckets < num_tuples) this->num_buckets <<= 1;
    }
    */
    this->buckets = new bucket[this->num_buckets];
  }
  // HashTable is Empty
  else
    this->buckets = nullptr;
}

hashTable::~hashTable() { delete[] buckets; }