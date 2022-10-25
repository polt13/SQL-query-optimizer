#include "hashTable.h"

tuple *bucket::getTuple() const { return this->mytuple; }

bool bucket::getOccupied() const { return this->occupied; }

bool bucket::getBitmapIndex(uint64_t index) const {
  return this->Bitmap[index];
}

// bucket *bucket::getBitmapBucket(uint64_t index) { return this->Bitmap[index];
// }

void bucket::setTuple(tuple *t) { this->mytuple = t; }

void bucket::setOccupied(bool flag) { this->occupied = flag; }

void bucket::setBitmapIndex(uint64_t index, bool flag) {
  this->Bitmap[index] = flag;
}

/* void bucket::setBitmapBucket(uint64_t index, bucket *mybucket) {
  this->Bitmap[index] = mybucket;
} */

bucket::bucket() : mytuple{nullptr}, occupied{false}, Bitmap{} {}

bucket::~bucket() {}

// ------------------------------------------------------------------

uint64_t hashTable::getBucketCount() const { return num_buckets; }

uint64_t hashTable::hash2(int64_t key) {
  // todo
  return key % this->num_buckets;
}

// Insert all tuples of a partition into the hashTable
void hashTable::insert(tuple *t) {
  uint64_t hashVal = hash2(t->getPayload());  // not sure if Key
  bool flag;

  // ----- Implement Hopscotch Hashing -----
  if (this->buckets[hashVal].getOccupied() == false) {
    this->buckets[hashVal].setTuple(t);
    this->buckets[hashVal].setOccupied(true);
    this->buckets[hashVal].setBitmapIndex(0, true);
  } else {
    // Check if Neighbourhood is FULL
    flag = true;  // Assume it's full
    for (uint64_t i = 0; i < NBHD_SIZE; i++) {
      if (this->buckets[hashVal].getBitmapIndex(i) == false) {
        flag = false;  // Neighbourhood NOT full
        break;
      }
    }
    // Step 1. FULL Neighbourhood
    if (flag == true) {
      // Rehash needed
      // call rehash function
      // insert(t);
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
      // call rehash function
      // insert(t);
    }
    // Step 3.
    while ((j - hashVal) % this->num_buckets >= NBHD_SIZE) {
      // Step 3.a.
      for (uint64_t k = j - NBHD_SIZE + 1; k < j; k++) {
        flag = false;
        // Search for an element in Neighbourhood
        for (uint64_t x = 0; x < NBHD_SIZE; x++) {
          if (this->buckets[k].getBitmapIndex(x) == true) {
            flag = true;  // Element found
            // Step 3.c
            this->buckets[j].setTuple(this->buckets[k + x].getTuple());
            this->buckets[j].setOccupied(true);
            this->buckets[k].setBitmapIndex((j - k), true);

            this->buckets[k].setBitmapIndex(x, false);
            this->buckets[k + x].setTuple(nullptr);
            this->buckets[k + x].setOccupied(false);
            // Step 3.d.
            j = k + x;
            break;
          }
        }
        // Step 3.b. | No element found
        if (flag == false) {
          // Rehash needed
          // call rehash function
          // insert(t);
          return;
        }
      }
    }
    // Step 4 | Save tuple here
    this->buckets[j].setTuple(t);
    this->buckets[j].setOccupied(true);
    this->buckets[hashVal].setBitmapIndex((j - hashVal), true);
  }
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
    for (uint64_t i = 0; i < NBHD_SIZE; i++) {
      if (this->buckets[hashVal + i].getTuple()->getKey() == key) {
        std::printf("Found item with key %ld\n", key);
        return;
      }
    }
    std::printf("Item with key %ld NOT FOUND\n", key);
  }
}

hashTable::hashTable(uint64_t num_tuples) : num_buckets{num_tuples} {
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

    /* for (uint64_t i = 0; i < this->num_buckets; i++) {
      uint64_t index = i;
      for (uint64_t y = 0; y < NBHD_SIZE; y++)
        // e.g. last bucket's bitmap points to nullptr(s)
        if (index < this->num_buckets) {
          this->buckets[i].setBitmapBucket(y, &buckets[index]);
          index++;
        }
    } */
  }
  // HashTable is Empty
  else
    this->buckets = nullptr;
}

hashTable::~hashTable() { delete[] buckets; }