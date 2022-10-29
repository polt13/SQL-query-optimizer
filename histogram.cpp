#include "histogram.h"

// create histogram with 2^n entries -- zero initialize all}
Histogram::Histogram(int64_t size)
    : entries{new int64_t[size]{}}, psum{new int64_t[size]{}}, size{size} {}

// increase value for a hashvalue
int64_t& Histogram::operator[](int64_t index) { entries[index]++; }

int64_t* Histogram::generatePsum() {
  int64_t start = 0;
  psum[0] = 0;
  for (int64_t p = 1; p < size; p++) {
    psum[p] = start + entries[p - 1];
    start += psum[p];
  }

  return psum;
}

int64_t Histogram::getPartitionEntries(int64_t index) const {
  return entries[index];
}

int64_t Histogram::getPartitionPsum(int64_t index) const { return psum[index]; }

int64_t Histogram::getSize() const { return size; }

Histogram::~Histogram() {
  delete[] entries;
  delete[] psum;
}