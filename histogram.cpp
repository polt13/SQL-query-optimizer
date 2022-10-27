#include "histogram.h"

Histogram::Histogram(int64_t size) {
  // create histogram with 2^n entries -- zero initialize all
  // each row  is basically a partition
  h = new HistEntry[size];
  entries = size;
}

const HistEntry& Histogram::getEntry(int64_t index) const { return h[index]; }

int64_t Histogram::getEntriesCount() const { return entries; }

// insert a tuple to the right partition (index are the final n bits)
void Histogram::insert(int64_t index, tuple t) { h[index].append(t); }

Histogram::~Histogram() { delete[] h; }