#include "dataForm.h"
#include "hashTable.h"
#include "partitioner.h"
#include "simple_vector.h"
#include "job_scheduler.h"

extern JobScheduler js;

result PartitionedHashJoin(relation& r, relation& s, int64_t forceDepth,
                           int64_t bits_pass1, int64_t bits_pass2) {
  // partitioning phase
  Partitioner rpartitioner, spartitioner;

  relation r_ = rpartitioner.partition(r, forceDepth, bits_pass1, bits_pass2);

  // force S relationship to be partitioned as many times as R was
  int64_t forcePartitioning = rpartitioner.getPartitioningLevel();
  relation s_ =
      spartitioner.partition(s, forcePartitioning, bits_pass1, bits_pass2);

  // use the vector inside for the results
  result result_join;

  // if partitioning has occured
  if (forcePartitioning != 0) {
    Histogram* rHist = rpartitioner.getHistogram();

    const int64_t* rpsum = rHist->getPsum();
    int64_t partitions = rHist->getSize();

    hashTable** partitionsHT = new hashTable*[partitions];

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

    result* thread_results = new result[partitions];

    for (int64_t j = 0; j < partitions; j++) {
      int64_t start = spsum[j];
      int64_t end = (j < (partitions - 1)) ? (spsum[j + 1]) : (s_.getAmount());

      js.add_job(
          new JoinJob(s_, start, end, partitionsHT[j], thread_results[j]));
    }

    js.wait_all();

    for (int64_t i = 0; i < partitions; i++) {
      for (int64_t j = 0; j < thread_results[i].getSize(); j++)
        result_join.push(thread_results[i][j]);
      delete partitionsHT[i];
    }
    delete[] thread_results;
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
          result_item entry{traverse->mytuple->getPayload(), s[j].getPayload()};
          result_join.push(entry);
          traverse = traverse->next;
        }
      }
    }
  }

  return result_join;
}

void joinBuckets(relation& s_, int64_t start, int64_t end,
                 hashTable* partitionHT, result& result_join) {
  for (int64_t k = start; k < end; k++) {
    List* tuple_list = partitionHT->findEntry(s_[k].getKey());
    if (tuple_list) {
      Node* traverse = tuple_list->getRoot();
      while (traverse) {
        // result is [rowid_r,rowid_s]
        result_item entry{traverse->mytuple->getPayload(), s_[k].getPayload()};
        result_join.push(entry);
        traverse = traverse->next;
      }
    }
  }
}