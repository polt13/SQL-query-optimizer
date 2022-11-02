#include "dataForm.h"
#include "partitioner.h"
#include "hashTable.h"

relation PartitionedHashJoin(relation& r, relation& s) {
  // partitioning phase
  Partitioner rpartitioner, spartitioner;

  relation r_ = rpartitioner.partition(r);

  // force S relationship to be partitioned as many times as R was
  int64_t forcePartitioning = rpartitioner.getPartitioningLevel();
  relation s_ = spartitioner.partition(s, forcePartitioning);

  Histogram* rHist = rpartitioner.getHistogram();

  const int64_t* rpsum = rHist->getPsum();
  int64_t partitions = rHist->getSize();

  hashTable** partitionsHT = new hashTable* [partitions] {};

  // build phase
  for (int64_t i = 0; i < partitions; i++) {
    int64_t entries = (*rHist)[i];
    // create hashtable with size =  as many as the partitions in the tuble

    partitionsHT[i] = new hashTable(entries);

    int64_t start = rpsum[i];
    std::printf("Partition starts at %ld\n", start);
    int64_t end =
        (i < (partitions - 1)) ? (rpsum[i + 1]) : (r_.getAmount() - 1);

    for (int64_t j = start; j < end; j++) partitionsHT[i]->insert(&r_[j]);
  }

  // placeholder
  return s_;
}