#ifndef SVECTOR
#define SVECTOR
#include <cstdio>
#include <cstring>
template <typename T>
class simple_vector {
  size_t capacity;
  size_t size;
  T* objarr;

 public:
  simple_vector(size_t start_capacity = 10)
      : capacity{start_capacity}, size{}, objarr{new T[capacity]} {}

  void add_back(const T& obj) {
    objarr[size++] = obj;
    // if capacity is maxed
    // increase capacity and copy objects to new location in memory
    if (size == capacity) {
      capacity = 2 * size;
      T* old = objarr;
      objarr = new T[capacity];

      std::memmove(objarr, old, sizeof(T) * size);

      delete[] old;
    }
  }

  T& operator[](int idx) { return objarr[idx]; }

  size_t getSize() const { return size; }

  simple_vector(simple_vector& other) {
    size = other.size;
    capacity = size * 2;
    objarr = new T[capacity];
    std::memmove(objarr, other.objarr, sizeof(T) * size);
  }

  ~simple_vector() { delete[] objarr; }

  simple_vector<T>& operator=(simple_vector& other) {
    // if the vector doesnt have enough memory allocated to fit the copied data
    //  allocate new memory and delete the old one
    // if (capacity < other.size) {
    //   delete[] objarr;
    //   objarr = new T[other.size + 1];
    // }

    // size = other.size;

    // std::memmove(objarr, other.objarr, sizeof(T) * size);

    objarr = other.objarr;
    capacity = other.capacity;
    size = other.size;
    other.objarr = nullptr;

    return *this;
  }

  bool find(T item) {
    for (size_t i = 0; i < size; i++)
      if (objarr[i] == item) return true;
    return false;
  }

  void clear() { this->size = 0; }
};
#endif