#include "hashTable.h"

tuple *bucket::getTuple() const { return this->mytuple; }

bool bucket::getFlag() const { return this->flag; }

bool bucket::getBitmapIndex(uint64_t index) const {
  return this->Bitmap[index];
}

// bucket *bucket::getBitmapBucket(uint64_t index) { return this->Bitmap[index];
// }

void bucket::setTuple(tuple *t) { this->mytuple = t; }

void bucket::setFlag(bool f) { this->flag = f; }

void bucket::setBitmapIndex(uint64_t index, bool b) { this->Bitmap[index] = b; }

/* void bucket::setBitmapBucket(uint64_t index, bucket *mybucket) {
  this->Bitmap[index] = mybucket;
} */

bucket::bucket() {
  this->mytuple = nullptr;
  this->flag = false;
}

bucket::~bucket() { delete mytuple; }

// ------------------------------------------------------------------

uint64_t hashTable::getBucketCount() const { return num_buckets; }

uint64_t hashTable::hash2(int64_t key) {
  // todo
  return 1;
}

// Insert all tuples of a partition into the hashTable
void hashTable::insert(tuple *t) {
  uint64_t hashVal = hash2(t->getKey());  // not sure if Key

  // ----- Implement Hopscotch Hashing -----
  if (this->buckets[hashVal].getFlag() == false) {
    this->buckets[hashVal].setTuple(t);
    this->buckets[hashVal].setFlag(true);
    this->buckets[hashVal].setBitmapIndex(0, true);
  } else {
    // Check if Neighbourhood is FULL
    bool flag = true;  // Assume it's full
    for (uint64_t i = 0; i < HBHD_SIZE; i++) {
      if (hashVal + i >= this->num_buckets) break;
      if (this->buckets[hashVal].getBitmapIndex(i) == false) {
        flag = false;  // Neighbourhood NOT full
        break;
      }
    }
    // Step 1. Empty slot NOT FOUND -
    if (flag == true) {
      // Rehash needed
      // call rehash function
      return;
    }
    // Step 2.
    for (uint64_t j = hashVal; j < HBHD_SIZE; j++) {  // Empty slot found
      if (this->buckets[j].getFlag() == false) {
        // Step 3.
        if ((j - hashVal) % this->num_buckets >= HBHD_SIZE) {
          // Step 3.a.
          // Step 3.b.
          // Step 3.c
          // Step 3.d.
          // Step 4
        } else {
          // Save tuple here
        }
      }
    }
  }

  // this->buckets[hashVal].setVal(t.getKey());
}

void hashTable::fillHT(const Partition &part) {
  Node *traverse = part.getPartitionList();
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
    for (uint64_t i = 0; i < HBHD_SIZE; i++) {
      if (this->buckets[hashVal + 0].getTuple()->getKey() == key) {
        std::printf("Found item with key %ld\n", key);
        return;
      }
    }
    std::printf("Item with key %ld NOT FOUND\n", key);
  }
}

hashTable::hashTable(uint64_t num_tuples) {
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
    this->num_buckets = num_tuples;
    this->buckets = new bucket[this->num_buckets];

    /* for (uint64_t i = 0; i < this->num_buckets; i++) {
      uint64_t index = i;
      for (uint64_t y = 0; y < HBHD_SIZE; y++)
        // e.g. last bucket's bitmap points to nullptr(s)
        if (index < this->num_buckets) {
          this->buckets[i].setBitmapBucket(y, &buckets[index]);
          index++;
        }
    } */
  }
  // HashTable is Empty
  else {
    this->num_buckets = 0;
    this->buckets = nullptr;
  }
}

hashTable::~hashTable() { delete[] buckets; }