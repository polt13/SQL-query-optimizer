#include "hashTable.h"

uint64_t bucket::getVal() const { return val; }

bool bucket::getFlag() const { return flag; }

bucket *bucket::getBitmapBucket(uint64_t index) { return Bitmap[index]; }

void bucket::setVal(uint64_t v) { val = v; }

void bucket::setFlag(bool f) { flag = f; }

void bucket::setBitmapBucket(uint64_t index, bucket *mybucket) {
  Bitmap[index] = mybucket;
}

bucket::bucket() { this->flag = false; }

// ------------------------------------------------------------------

uint64_t hashTable::getBucketCount() const { return num_buckets; }

uint64_t hashTable::hash2(uint64_t key) {
  // todo
  return 1;
}

// Insert all tuples of a partition into the hashTable
void hashTable::insert(const tuple &t) {
  uint64_t hashVal = hash2(t.getKey());  // not sure if Key
                                         // implement hopscotch hashing
  // this->buckets[hashVal].setVal(t.getKey());
}

void hashTable::fillHT(const Partition &part) {
  Node *traverse = part.getPartitionList();
  while (traverse) {
    insert(traverse->t);
    traverse = traverse->next;
  }
}

hashTable::hashTable(uint64_t num_tuples) {
  if (num_tuples > 0) {
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
    this->buckets = new bucket[this->num_buckets];

    for (uint64_t i = 0; i < num_buckets; i++) {
      uint64_t index = i;
      for (uint64_t y = 0; y < H; y++)
        if (index < num_buckets) {
          this->buckets[i].setBitmapBucket(y, &buckets[index]);
          index++;
        }
    }
  }
}