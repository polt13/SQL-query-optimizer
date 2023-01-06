#ifndef SIMPLE_HT
#define SIMPLE_HT
#include "simple_vector.h"
#define NBHD_SIZE 32
#include <cstdint>
#define LF 2.0
#include <cstdlib>
#include <cstdio>

int64_t _hash(const int &key) { return key; }

int64_t _hash2(const char *const &key) {
  int64_t sum = 0;
  const char *t = key;
  while (*t) sum += *(t++);

  return sum;
}

template <typename U, typename R>
class entry {
  U key;
  R item;

 public:
  U &get_key() { return key; }

  R &get_val() { return item; }

  R &set_val(const R &i) { return item = i; }

  entry() = default;

  entry(const U &key, const R &item) : key{key}, item{item} {};
};

// U = key
// T  = val
template <typename U, typename T>
class simple_ht {
  // array of vectors, each vector has the values/keys
  int64_t num_buckets;
  simple_vector<entry<U, T>> *buckets;
  int64_t elems;
  int64_t (*hash)(const U &key);

  T &insert(int64_t index, const U &key, const T &val) {
    ++elems;

    if (((double)elems / num_buckets) > LF) return rehash(key, val);

    simple_vector<entry<U, T>> &target_bucket = buckets[index];

    target_bucket.add_back(entry<U, T>{key, val});

    return target_bucket[target_bucket.getSize() - 1].get_val();
  }

  T &rehash(const U &key, const T &val) {
    int64_t old_bucket_count = num_buckets;

    simple_vector<entry<U, T>> *old_buckets = buckets;

    num_buckets = old_bucket_count * 2;

    buckets = new simple_vector<entry<U, T>>[num_buckets];

    for (int64_t i = 0; i < old_bucket_count; i++) {
      simple_vector<entry<U, T>> &old_bucket_items = old_buckets[i];
      for (int64_t j = 0; j < old_bucket_items.getSize(); j++) {
        U &_key = old_bucket_items[j].get_key();
        T &_val = old_bucket_items[j].get_val();
        int64_t index = this->hash(_key) % num_buckets;
        buckets[index].add_back(entry<U, T>{_key, _val});
      }
    }

    // insert new element

    int64_t new_index = this->hash(key) % num_buckets;

    buckets[new_index].add_back(entry<U, T>{key, val});

    delete[] old_buckets;

    return buckets[new_index][buckets[new_index].getSize() - 1].get_val();
  }

 public:
  T &operator[](const U &key) {
    int64_t index = this->hash(key) % num_buckets;

    simple_vector<entry<U, T>> &target_bucket = buckets[index];

    for (size_t i = 0; i < target_bucket.getSize(); i++) {
      if (target_bucket[i].get_key() == key) return target_bucket[i].get_val();
    }

    // if element doesn't exist, add dummy element and return reference to
    // that
    return insert(index, key, T());
  }

  simple_ht(int64_t (*_fn)(const U &), int64_t buckets = 8)
      : num_buckets{buckets},
        buckets{new simple_vector<entry<U, T>>[num_buckets]},
        elems{},
        hash{_fn} {}

  ~simple_ht() { delete[] buckets; }
};

#endif
