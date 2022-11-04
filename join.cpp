#include "dataForm.h"
#include "partitioner.h"
#include "hashTable.h"

result PartitionedHashJoin(relation& r, relation& s, int64_t forceDepth,
                           int64_t bits_pass1, int64_t bits_pass2) {
  // partitioning phase
  Partitioner rpartitioner, spartitioner;

  relation r_ = rpartitioner.partition(r, forceDepth, bits_pass1, bits_pass2);

  // force S relationship to be partitioned as many times as R was
  int64_t forcePartitioning = rpartitioner.getPartitioningLevel();
  relation s_ =
      spartitioner.partition(s, forcePartitioning, bits_pass1, bits_pass2);

  // the result is at most as big as the largest relation
  int64_t maxResult =
      (r_.getAmount() > s_.getAmount()) ? (r_.getAmount()) : (s_.getAmount());

  result_item* result_join = new result_item[maxResult];
  int64_t result_size = 0;

  // if partitioning has occured
  if (forcePartitioning != 0) {
    Histogram* rHist = rpartitioner.getHistogram();

    const int64_t* rpsum = rHist->getPsum();
    int64_t partitions = rHist->getSize();

    // initialize all to nullptr
    hashTable** partitionsHT = new hashTable* [partitions] {};

    // build phase
    for (int64_t i = 0; i < partitions; i++) {
      int64_t entries = (*rHist)[i];
      // create hashtable with size =  as many as the partitions in the tuple
      partitionsHT[i] = new hashTable(entries);

      int64_t start = rpsum[i];
      int64_t end = (i < (partitions - 1)) ? (rpsum[i + 1]) : (r_.getAmount());

      for (int64_t j = start; j < end; j++) partitionsHT[i]->insert(&r_[j]);
    }

    Histogram* sHist = spartitioner.getHistogram();
    const int64_t* spsum = sHist->getPsum();

    for (int64_t j = 0; j < partitions; j++) {
      int64_t start = spsum[j];
      int64_t end = (j < (partitions - 1)) ? (spsum[j + 1]) : (s_.getAmount());

      for (int64_t k = start; k < end; k++) {
        List* tuple_list = partitionsHT[j]->findEntry(s_[k].getKey());
        if (tuple_list) {
          Node* traverse = tuple_list->getRoot();
          while (traverse) {
            // result is [tuple_r,tuple_s]
            result_join[result_size++] = {*(traverse->mytuple), s_[k]};
            traverse = traverse->next;
          }
        }
      }
    }

    for (int64_t i = 0; i < partitions; i++) {
      delete partitionsHT[i];
    }
    delete[] partitionsHT;

  }
  // no partitioning case
  else {
    int64_t r_entries = r.getAmount();
    int64_t s_entries = s.getAmount();
    hashTable h{r_entries};
    for (int64_t i = 0; i < r_entries; i++) h.insert(&r[i]);
    for (int64_t j = 0; j < s_entries; j++) {
      List* tuple_list = h.findEntry(s[j].getKey());
      if (tuple_list) {
        Node* traverse = tuple_list->getRoot();
        while (traverse) {
          result_join[result_size++] = {*(traverse->mytuple), s[j]};
          traverse = traverse->next;
        }
      }
    }
  }

  return result(result_join, result_size);
}