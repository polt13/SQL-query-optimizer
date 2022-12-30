#include "partitioner.h"
#include "job_scheduler.h"
#include <cstdio>
#include <cstring>

extern JobScheduler js;

/*
 * Hash Function for partitioning
 * Get the n Least Significant Bits (LSB)
 */
int64_t Partitioner::hash1(uint64_t key, uint64_t n) {
  uint64_t num = 1;
  num <<= n;
  // e.g. n = 3
  // 1000 - 1 = 111
  // val = key & (2^n - 1); // bitwise AND
  return key & (num - 1);
}

relation Partitioner::partition1(relation& r, int64_t bits_pass1) {
  int64_t r_entries = r.getAmount();
  // if partitioning is needed at least once
  // std::printf("\nOne pass needed\n");
  partitioningLevel = 1;

  int64_t partitions = 1 << bits_pass1;

  // main hist
  hist = new Histogram{partitions};

  Histogram* hvector[THREAD_COUNT];

  int64_t per_thread = r_entries / THREAD_COUNT;

  for (int64_t i = 0; i < THREAD_COUNT; i++) {
    int64_t start = i * per_thread;
    int64_t end = (i + 1) * per_thread;
    if ((i == THREAD_COUNT - 1) && (end < r_entries)) end = r_entries;
    js.add_job(new HistogramJob(r, start, end, hvector[i], bits_pass1));
  }

  js.wait_all();

  for (int64_t i = 0; i < THREAD_COUNT; i++) {
    for (int64_t j = 0; j < partitions; j++) {
      (*hist)[j] += (*(hvector[i]))[j];
    }

    delete hvector[i];
  }

  const int64_t* psum = hist->generatePsum();

  // create copy so that we know where the next item needs to go
  // psum_copy is being mutated
  int64_t* psum_copy = new int64_t[hist->getSize()];
  std::memmove(psum_copy, psum, hist->getSize() * sizeof(int64_t));

  // sorted r'
  relation r2(new tuple[r_entries], r_entries);

  for (int64_t t = 0; t < r_entries; t++) {
    int64_t index = hash1(r[t].getKey(), bits_pass1);
    int64_t insertTo = psum_copy[index];

    r2[insertTo] = r[t];
    // indicate that there's one less tuple to copy
    // move index forward
    psum_copy[index]++;
  }

  delete[] psum_copy;

  return r2;
}

relation Partitioner::partition2(relation& r2, int64_t bits_pass2) {
  // std::printf("Second pass needed\n");
  partitioningLevel = 2;

  int64_t r2_entries = r2.getAmount();

  // discard old histogram, create a new using n2
  Histogram* oldHist = hist;

  int64_t partitions2 = 1 << bits_pass2;

  hist = new Histogram(partitions2);

  Histogram* hvector[THREAD_COUNT];

  // do the same thing, using r2

  int64_t per_thread = r2_entries / THREAD_COUNT;

  for (int64_t i = 0; i < THREAD_COUNT; i++) {
    int64_t start = i * per_thread;
    int64_t end = (i + 1) * per_thread;
    if ((i == THREAD_COUNT - 1) && (end < r2_entries)) end = r2_entries;
    js.add_job(new HistogramJob(r2, start, end, hvector[i], bits_pass2));
  }

  js.wait_all();

  for (int64_t i = 0; i < THREAD_COUNT; i++) {
    for (int64_t j = 0; j < partitions2; j++) {
      (*hist)[j] += (*(hvector[i]))[j];
    }
    delete hvector[i];
  }

  const int64_t* psum = hist->generatePsum();

  int64_t* psum_copy = new int64_t[hist->getSize()];
  std::memmove(psum_copy, psum, hist->getSize() * sizeof(int64_t));

  relation r3(new tuple[r2_entries], r2_entries);

  for (int64_t t = 0; t < r2_entries; t++) {
    int64_t index = hash1(r2[t].getKey(), bits_pass2);
    int64_t insertTo = psum_copy[index];
    r3[insertTo] = r2[t];
    psum_copy[index]++;
  }

  // delete histogram from pass 1
  delete oldHist;

  delete[] psum_copy;

  // if reordered once more, return final
  return r3;
}

relation Partitioner::partition(relation& r, int64_t force_partition_depth,
                                int64_t bits_pass1, int64_t bits_pass2) {
  if (force_partition_depth == 0) {
    return r;
  } else if (force_partition_depth == 1) {
    return partition1(r, bits_pass1);
  } else if (force_partition_depth == 2) {
    return partition2(r, bits_pass2);
  }

  if (((r.getAmount() * sizeof(tuple)) < L2_SIZE)) {
    // std::printf("Doesn't need partitioning\n");
    return r;
  }

  relation r2 = partition1(r, bits_pass1);

  int64_t partitions = hist->getSize();

  for (int64_t i = 0; i < partitions; i++) {
    if (((*hist)[i] * sizeof(tuple)) > L2_SIZE) {
      return partition2(r2, bits_pass2);
    }
  }

  // if partitions fit

  return r2;
}

// 2^n sized histogram
Partitioner::Partitioner() : hist{nullptr}, partitioningLevel{} {}

Partitioner::~Partitioner() { delete hist; }

int64_t Partitioner::getPartitioningLevel() const { return partitioningLevel; }

Histogram* Partitioner::getHistogram() const { return hist; }
